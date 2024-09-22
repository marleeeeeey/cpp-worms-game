#pragma once
#include <SDL_render.h>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <utils/animation.h>
#include <utils/level_info.h>
#include <utils/resources/resource_cache.h>
#include <utils/sdl/sdl_RAII.h>
#include <utils/sdl/sdl_colors.h>

// High-level resource management. Get resources by friendly names in game like terminolgy.
// Every Get* method define specific resource type and return it by friendly name.
// Example of resource map file `resourceMapFilePath` content:
// {
//   "animations": {
//     "playerWalk": "animations/playerWalk.json"
//   },
//   "sounds": {
//     "background_music": "path/to/sounds/background_music.ogg",
//     "explosion": "path/to/sounds/explosion.wav"
//   },
//   "textures": {
//     "player_texture": "path/to/textures/player.png",
//     "enemy_texture": "path/to/textures/enemy.png"
//   },
//   "maps": {
//     "level1": "path/to/maps/level1.json",
//     "level2": "path/to/maps/level2.json"
//   }
// }
class ResourceManager
{
public:
    struct SoundEffectInfo
    {
        std::shared_ptr<SoundEffectRAII> soundEffect;
        float volumeShift = 0.0f;
    };
private:
    struct SoundEffectBatch
    {
        std::vector<std::filesystem::path> paths;
        float volumeShift = 0.0f;
    };
    details::ResourceCache resourceCashe;
    using FriendlyName = std::string;
    using TagToAnimationDict = std::unordered_map<FriendlyName, Animation>;
    std::unordered_map<FriendlyName, TagToAnimationDict> animations;
    std::unordered_map<FriendlyName, LevelInfo> tiledLevels;
    std::unordered_map<FriendlyName, std::filesystem::path> musicPaths;
    std::unordered_map<std::string, std::vector<SoundEffectBatch>> soundEffectBatchesPerTag;
public:
    ResourceManager(SDL_Renderer* renderer, const nlohmann::json& assetsSettingsJson);
public: // //////////////////////////////////////// Animations ////////////////////////////////////////
    enum class TagProps
    {
        ExactMatch,
        RandomByRegex
    };
    // Get animation by name without tag. Load the first tag found.
    Animation GetAnimation(const std::string& animationName);
    Animation GetAnimation(const std::string& animationName, const std::string& tagName, TagProps tagProps = TagProps::ExactMatch);
private:
    Animation GetAnimationExactMatch(const std::string& animationName, const std::string& tagName);
    Animation GetAnimationByRegexRandomly(const std::string& animationName, const std::string& regexTagName);
    TagToAnimationDict ReadAsepriteAnimation(const std::filesystem::path& asepriteAnimationJsonPath);
public: // //////////////////////////////////////// Tiled levels ////////////////////////////////////////
    LevelInfo GetTiledLevel(const std::string& name);
public: // ////////////////////////////////////////// Textures //////////////////////////////////////////
    std::shared_ptr<SDLTextureRAII> GetColoredPixelTexture(ColorName color);
    std::shared_ptr<SDLTextureRAII> GetTexture(const std::filesystem::path& path);
    std::shared_ptr<SDLSurfaceRAII> GetSurface(const std::filesystem::path& path);
public: // /////////////////////////////////////////// Sounds ///////////////////////////////////////////
    std::shared_ptr<MusicRAII> GetMusic(const std::string& name);
    SoundEffectInfo GetSoundEffect(const std::string& name);
};