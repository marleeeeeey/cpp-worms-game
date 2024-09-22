#include "resource_manager.h"
#include <SDL_rect.h>
#include <cstddef>
#include <filesystem>
#include <glob/glob.hpp>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/dict_utils.h>
#include <my_cpp_utils/json_utils.h>
#include <my_cpp_utils/math_utils.h>
#include <my_cpp_utils/string_utils.h>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>
#include <utils/logger.h>
#include <utils/resources/aseprite_data.h>
#include <utils/resources/resource_cache.h>
#include <utils/sdl/sdl_texture_process.h>

ResourceManager::ResourceManager(SDL_Renderer* renderer, const nlohmann::json& assetsSettingsJson) : resourceCashe(renderer)
{
    // Load animations.
    for (const auto& animationPair : assetsSettingsJson["animations"].items())
    {
        const std::string& animationName = animationPair.key();
        auto animationPath = animationPair.value().get<std::filesystem::path>();
        animations[animationName] = ReadAsepriteAnimation(animationPath);
    }

    // Load tiled level names.
    for (const auto& tiledLevelPair : assetsSettingsJson["maps"].items())
    {
        LevelInfo levelInfo = tiledLevelPair.value().get<LevelInfo>();
        if (!std::filesystem::exists(levelInfo.tiledMapPath))
            throw std::runtime_error(MY_FMT("Tiled level file does not found: {}", levelInfo.tiledMapPath));
        tiledLevels[levelInfo.name] = levelInfo;
    }

    // Load sound effects.
    for (const auto& soundEffectPair : assetsSettingsJson["sound_effects"].items())
    {
        // "explosion"
        const std::string& soundEffectName = soundEffectPair.key();
        // array of { "glob": "assets/sound_effects/explosion*.wav", "volumeShift": 0.5 }
        const auto& globAndVolumeShiftList = soundEffectPair.value();

        if (!globAndVolumeShiftList.is_array())
            throw std::runtime_error(MY_FMT("Sound effect paths for '{}' should be an array", soundEffectName));

        std::vector<SoundEffectBatch> soundEffectInfos;

        // Load sound effect paths.
        for (const auto& globAndVolumeShift : globAndVolumeShiftList)
        {
            auto globIt = globAndVolumeShift.find("glob");
            if (globIt == globAndVolumeShift.end())
                throw std::runtime_error("Sound effect path should have 'glob' field");

            // "assets/sound_effects/explosion*.wav"
            auto globPath = globIt->get<std::string>();

            auto volumeShiftIt = globAndVolumeShift.find("volumeShift");
            float volumeShift = volumeShiftIt != globAndVolumeShift.end() ? volumeShiftIt->get<float>() : 0.0f;

            SoundEffectBatch soundEffectInfo;
            soundEffectInfo.volumeShift = volumeShift;

            for (auto& soundEffectPath : glob::glob(globPath))
            {
                soundEffectInfo.paths.push_back(soundEffectPath);
            }

            soundEffectInfos.push_back(soundEffectInfo);
        }
        MY_LOG(debug, "Sound effect '{}' has {} batch(es)", soundEffectName, soundEffectInfos.size());
        soundEffectBatchesPerTag[soundEffectName] = soundEffectInfos;
    }

    // Load music.
    for (const auto& musicPair : assetsSettingsJson["music"].items())
    {
        const std::string& musicName = musicPair.key();
        const auto musicPath = musicPair.value().get<std::filesystem::path>();
        musicPaths[musicName] = musicPath;
    }

    MY_LOG(
        info, "Game found {} animation(s), {} level(s), {} music(s), {} sound effect(s).", animations.size(), tiledLevels.size(), musicPaths.size(),
        soundEffectBatchesPerTag.size());
}

Animation ResourceManager::GetAnimation(const std::string& animationName)
{
    if (!animations.contains(animationName))
        throw std::runtime_error(MY_FMT("Animation with name '{}' does not found", animationName));

    if (animations[animationName].size() != 1)
        throw std::runtime_error(MY_FMT("Animation with name '{}' has more than one tag", animationName));

    // Get first animation tag.
    return animations[animationName].begin()->second;
}

Animation ResourceManager::GetAnimation(const std::string& animationName, const std::string& tagName, TagProps tagProps)
{
    if (tagProps == TagProps::ExactMatch)
        return GetAnimationExactMatch(animationName, tagName);

    if (tagProps == TagProps::RandomByRegex)
        return GetAnimationByRegexRandomly(animationName, tagName);

    throw std::runtime_error(MY_FMT("Unknown TagProps: {}", static_cast<int>(tagProps)));
}

Animation ResourceManager::GetAnimationExactMatch(const std::string& animationName, const std::string& tagName)
{
    if (!animations.contains(animationName))
        throw std::runtime_error(MY_FMT("Animation with name '{}' does not found", animationName));

    if (!animations[animationName].contains(tagName))
        throw std::runtime_error(MY_FMT("Animation tag with name '{}' does not found in {}", tagName, animationName));

    return animations[animationName][tagName];
}

Animation ResourceManager::GetAnimationByRegexRandomly(const std::string& animationName, const std::string& regexTagName)
{
    if (!animations.contains(animationName))
        throw std::runtime_error(MY_FMT("Animation with name '{}' does not found", animationName));

    std::vector<std::string> foundTags;
    for (const auto& [tag, _] : animations[animationName])
    {
        if (std::regex_match(tag, std::regex(regexTagName)))
            foundTags.push_back(tag);
    }

    if (foundTags.empty())
        throw std::runtime_error(MY_FMT("Animation tag with regex '{}' does not found in {}", regexTagName, animationName));

    auto randomTagOpt = utils::RandomIndexOpt(foundTags);
    return animations[animationName][foundTags[randomTagOpt.value()]];
}

namespace
{

AnimationFrame GetAnimationFrameFromAsepriteFrame(const AsepriteData::Frame& asepriteFrame, std::shared_ptr<SDLTextureRAII> textureRAII)
{
    AnimationFrame animationFrame;
    animationFrame.tileComponent.texturePtr = textureRAII;
    animationFrame.tileComponent.textureRect = asepriteFrame.rectInTexture;
    animationFrame.tileComponent.sizeWorld = {asepriteFrame.rectInTexture.w, asepriteFrame.rectInTexture.h};
    animationFrame.duration = asepriteFrame.duration_seconds;
    return animationFrame;
}

} // namespace

ResourceManager::TagToAnimationDict ResourceManager::ReadAsepriteAnimation(const std::filesystem::path& asepriteAnimationJsonPath)
{
    auto asepriteJsonData = utils::LoadJsonFromFile(asepriteAnimationJsonPath);

    AsepriteData asepriteData;
    try
    {
        asepriteData = LoadAsepriteData(asepriteJsonData);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(MY_FMT("[ReadAsepriteAnimation] Failed to load Aseprite data from '{}': {}", asepriteAnimationJsonPath, e.what()));
    }

    // Load texture.
    auto animationTexturePath = asepriteAnimationJsonPath.parent_path() / asepriteData.texturePath;
    std::shared_ptr<SDLTextureRAII> textureRAII = resourceCashe.LoadTexture(animationTexturePath);
    // Surface with sreaming access is needed to get hitbox rect.
    std::shared_ptr<SDLSurfaceRAII> surfaceRAII = resourceCashe.LoadSurface(animationTexturePath);

    TagToAnimationDict tagToAnimationDict;

    if (!asepriteData.frameTags.empty())
    {
        std::optional<SDL_Rect> hitboxRect;
        if (asepriteData.frameTags.contains("Hitbox"))
        {
            const SDL_Rect& rectInSurface = asepriteData.frames[asepriteData.frameTags["Hitbox"].from].rectInTexture;
            hitboxRect = GetVisibleRectInSrcRectCoordinates(surfaceRAII->get(), rectInSurface);
            MY_LOG(debug, "Hitbox rect found: x={}, y={}, w={}, h={}", hitboxRect->x, hitboxRect->y, hitboxRect->w, hitboxRect->h);
        }

        for (const auto& [_, frameTag] : asepriteData.frameTags)
        {
            if (frameTag.name == "Hitbox")
                continue; // Skip hitbox tag (it's not an animation tag, it's a tag for hitbox frame).

            Animation animation;
            animation.hitboxRect = hitboxRect;
            for (size_t i = frameTag.from; i <= frameTag.to; ++i)
            {
                AnimationFrame animationFrame = GetAnimationFrameFromAsepriteFrame(asepriteData.frames[i], textureRAII);

                MY_LOG(
                    debug, "Frame {} has texture rect: x={}, y={}, w={}, h={}", i, animationFrame.tileComponent.textureRect.x, animationFrame.tileComponent.textureRect.y,
                    animationFrame.tileComponent.textureRect.w, animationFrame.tileComponent.textureRect.h);

                animation.frames.push_back(std::move(animationFrame));
            }
            tagToAnimationDict[frameTag.name] = animation;
        }
    }
    else
    {
        // If there are no tags, then create one animation with all frames.
        Animation animation;
        for (size_t i = 0; i < asepriteData.frames.size(); ++i)
        {
            AnimationFrame animationFrame = GetAnimationFrameFromAsepriteFrame(asepriteData.frames[i], textureRAII);
            animation.frames.push_back(std::move(animationFrame));
        }
        tagToAnimationDict[""] = animation;
    }

    // Log names of loaded animations and tags.
    MY_LOG(debug, "Loaded animation from '{}': {}", asepriteAnimationJsonPath.string(), utils::JoinStrings(utils::GetKeys(tagToAnimationDict), ", "));
    for (const auto& [tag, animation] : tagToAnimationDict)
    {
        MY_LOG(debug, "  Tag '{}' has {} frame(s), hitbox rect found: {}", tag, animation.frames.size(), animation.hitboxRect.has_value());
    }

    return tagToAnimationDict;
}

LevelInfo ResourceManager::GetTiledLevel(const std::string& name)
{
    if (!tiledLevels.contains(name))
        throw std::runtime_error(MY_FMT("Tiled level with name '{}' does not found", name));
    return tiledLevels[name];
}

std::shared_ptr<SDLTextureRAII> ResourceManager::GetTexture(const std::filesystem::path& path)
{
    return resourceCashe.LoadTexture(path);
}

std::shared_ptr<MusicRAII> ResourceManager::GetMusic(const std::string& name)
{
    if (!musicPaths.contains(name))
        throw std::runtime_error(MY_FMT("Music with name '{}' does not found", name));
    return resourceCashe.LoadMusic(musicPaths[name]);
}

ResourceManager::SoundEffectInfo ResourceManager::GetSoundEffect(const std::string& name)
{
    if (!soundEffectBatchesPerTag.contains(name))
        throw std::runtime_error(MY_FMT("Sound effect with name '{}' does not found", name));

    // Get random sound effect from the list.
    const auto& soundEffectBatches = soundEffectBatchesPerTag[name];
    auto batchNumberOpt = utils::RandomIndexOpt(soundEffectBatches);
    if (!batchNumberOpt.has_value())
        throw std::runtime_error(MY_FMT("Sound effect batch for '{}' is empty", name));
    const SoundEffectBatch& soundEffectBatch = soundEffectBatches[batchNumberOpt.value()];
    auto trackNumberOpt = utils::RandomIndexOpt(soundEffectBatch.paths);
    if (!trackNumberOpt.has_value())
        throw std::runtime_error(MY_FMT("Sound effect track number for '{}' is empty", name));
    const auto& soundEffectPath = soundEffectBatch.paths[trackNumberOpt.value()];

    SoundEffectInfo soundEffectInfo;
    soundEffectInfo.soundEffect = resourceCashe.LoadSoundEffect(soundEffectPath);
    soundEffectInfo.volumeShift = soundEffectBatch.volumeShift;
    return soundEffectInfo;
}

std::shared_ptr<SDLSurfaceRAII> ResourceManager::GetSurface(const std::filesystem::path& path)
{
    return resourceCashe.LoadSurface(path);
}

std::shared_ptr<SDLTextureRAII> ResourceManager::GetColoredPixelTexture(ColorName color)
{
    return resourceCashe.GetColoredPixelTexture(color);
}