#include "resource_manager.h"
#include "utils/resource_cache.h"
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <glob/glob.hpp>
#include <my_common_cpp_utils/config.h>
#include <my_common_cpp_utils/logger.h>
#include <my_common_cpp_utils/math_utils.h>
#include <nlohmann/json.hpp>

ResourceManager::ResourceManager(SDL_Renderer* renderer) : resourceCashe(renderer)
{
    auto& assetsJson = utils::GetConfig<nlohmann::json, "assets">();

    // Load animations.
    for (const auto& animationPair : assetsJson["animations"].items())
    {
        const std::string& animationName = animationPair.key();
        auto animationPath = animationPair.value().get<std::filesystem::path>();
        animations[animationName] = ReadAsepriteAnimation(animationPath);
    }

    // Load tiled level names.
    for (const auto& tiledLevelPair : assetsJson["maps"].items())
    {
        const std::string& tiledLevelName = tiledLevelPair.key();
        LevelInfo levelInfo = tiledLevelPair.value().get<LevelInfo>();
        if (!std::filesystem::exists(levelInfo.tiledMapPath))
            throw std::runtime_error(MY_FMT("Tiled level file does not found: {}", levelInfo.tiledMapPath.string()));
        tiledLevels[levelInfo.name] = levelInfo;
    }

    // Load sound effects.
    for (const auto& soundEffectPair : assetsJson["sound_effects"].items())
    {
        const std::string& soundEffectName = soundEffectPair.key();
        const auto& soundEffectGlobsJson = soundEffectPair.value();

        if (!soundEffectGlobsJson.is_array())
            throw std::runtime_error(MY_FMT("Sound effect paths for '{}' should be an array", soundEffectName));

        std::vector<std::filesystem::path> paths;

        // Load sound effect paths.
        for (const auto& soundEffectGlobPath : soundEffectGlobsJson)
        {
            for (auto& soundEffectPath : glob::glob(soundEffectGlobPath.get<std::string>()))
            {
                paths.push_back(soundEffectPath);
            }
        }

        soundEffectPaths[soundEffectName] = paths;
    }

    // Load music.
    for (const auto& musicPair : assetsJson["music"].items())
    {
        const std::string& musicName = musicPair.key();
        const auto musicPath = musicPair.value().get<std::filesystem::path>();
        musicPaths[musicName] = musicPath;
    }

    MY_LOG_FMT(
        info, "Game found {} animation(s), {} level(s), {} music(s), {} sound effect(s).", animations.size(),
        tiledLevels.size(), musicPaths.size(), soundEffectPaths.size());
}

AnimationInfo ResourceManager::GetAnimation(const std::string& name)
{
    if (!animations.contains(name))
        throw std::runtime_error(MY_FMT("Animation with name '{}' does not found", name));
    return animations[name];
}

/**
 * Load aseprite animation from a json file. Example of the json file:
 * {
 *   "frames": {
 *     "m_walk 0.png": {
 *       "frame": { "x": 0, "y": 0, "w": 9, "h": 19 },
 *       ...
 *       "duration": 100
 *     },
 *     ...
 *   },
 *   "meta": {
 *     ...
 *     "image": "SPRITE-SHEET.png",
 *     ...
 *   }
 * }
 */
AnimationInfo ResourceManager::ReadAsepriteAnimation(const std::filesystem::path& asepriteAnimationJsonPath)
{
    // Read aseprite animation json file.
    std::ifstream asepriteAnimationJsonFile(asepriteAnimationJsonPath);
    if (!asepriteAnimationJsonFile.is_open())
    {
        throw std::runtime_error(
            MY_FMT("Failed to open aseprite animation file: {}", asepriteAnimationJsonPath.string()));
    }
    nlohmann::json asepriteJsonData;
    asepriteAnimationJsonFile >> asepriteJsonData;

    // Load texture.
    const std::string imagePath = asepriteJsonData["meta"]["image"].get<std::string>();
    auto animationTexturePath = asepriteAnimationJsonPath.parent_path() / imagePath;
    auto textureRAII = resourceCashe.LoadTexture(animationTexturePath);

    // Create animation.
    AnimationInfo animation;
    for (const auto& framePair : asepriteJsonData["frames"].items())
    {
        // Read frame info.
        const auto& frame = framePair.value()["frame"];
        int x = frame["x"];
        int y = frame["y"];
        int w = frame["w"];
        int h = frame["h"];
        int duration = framePair.value()["duration"];

        // Create animation frame.
        AnimationFrame animationFrame;
        animationFrame.renderingInfo.texturePtr = textureRAII;
        animationFrame.renderingInfo.textureRect = {x, y, w, h};
        animationFrame.renderingInfo.sdlSize = {w, h}; // TODO2: obsolete. Remove later.
        animationFrame.duration = static_cast<float>(duration) / 1000.0f; // Convert to seconds.

        // Add frame to the animation.
        animation.frames.push_back(std::move(animationFrame));
    }
    return animation;
};

LevelInfo ResourceManager::GetTiledLevel(const std::string& name)
{
    if (!tiledLevels.contains(name))
        throw std::runtime_error(MY_FMT("Tiled level with name '{}' does not found", name));
    return tiledLevels[name];
}

std::shared_ptr<SDLTextureRAII> ResourceManager::GetTexture(const std::filesystem::path& path)
{
    return resourceCashe.LoadTexture(path);
};

std::shared_ptr<MusicRAII> ResourceManager::GetMusic(const std::string& name)
{
    if (!musicPaths.contains(name))
        throw std::runtime_error(MY_FMT("Music with name '{}' does not found", name));
    return resourceCashe.LoadMusic(musicPaths[name]);
};

std::shared_ptr<SoundEffectRAII> ResourceManager::GetSoundEffect(const std::string& name)
{
    if (!soundEffectPaths.contains(name))
        throw std::runtime_error(MY_FMT("Sound effect with name '{}' does not found", name));

    // Get random sound effect from the list.
    const auto& sounds = soundEffectPaths[name];
    auto number = utils::Random<size_t>(0, soundEffectPaths[name].size() - 1);
    return resourceCashe.LoadSoundEffect(sounds[number]);
};

std::shared_ptr<SDLSurfaceRAII> ResourceManager::GetSurface(const std::filesystem::path& path)
{
    return resourceCashe.LoadSurface(path);
};

std::shared_ptr<SDLTextureRAII> ResourceManager::GetColoredPixelTexture(ColorName color)
{
    return resourceCashe.GetColoredPixelTexture(color);
};