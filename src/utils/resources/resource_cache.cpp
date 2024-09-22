#include "resource_cache.h"
#include <filesystem>
#include <utils/logger.h>
#include <utils/sdl/sdl_RAII.h>
#include <utils/sdl/sdl_colors.h>
#include <utils/sdl/sdl_texture_process.h>

namespace details
{

namespace
{

SDLTextureRAII GetColoredPixelTexture(SDL_Renderer* renderer, const SDL_Color& color)
{
    SDLTextureRAII texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 1, 1);

    SDLTextureLockRAII lock(texture.get());
    auto pixels = lock.GetPixels();

    // Set the pixel to the specified color.
    Uint32* formattedPixels = static_cast<Uint32*>(pixels);
    *formattedPixels = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), color.r, color.g, color.b, color.a);

    return texture;
}

SDLTextureRAII GetColoredPixelTexture(SDL_Renderer* renderer, const ColorName& color)
{
    return GetColoredPixelTexture(renderer, GetSDLColor(color));
}

} // namespace

ResourceCache::ResourceCache(SDL_Renderer* renderer) : renderer(renderer)
{}

std::shared_ptr<SDLTextureRAII> ResourceCache::LoadTexture(const std::filesystem::path& filePath)
{
    // Get absolute path to the file.
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);

    // Return cached texture if it was already loaded.
    if (textures.contains(absolutePath))
        return textures[absolutePath];

    MY_LOG(debug, "Loading texture: {}", filePath.string());

    // Load the texture and cache it.
    std::shared_ptr<SDLTextureRAII> textureRAII;
    textureRAII = details::LoadTexture(renderer, absolutePath);
    textures[absolutePath] = textureRAII;
    return textureRAII;
}

std::shared_ptr<MusicRAII> ResourceCache::LoadMusic(const std::filesystem::path& filePath)
{
    // Get absolute path to the file.
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);

    // Return cached music if it was already loaded.
    if (musics.contains(absolutePath))
        return musics[absolutePath];

    // Load the music and cache it.
    std::shared_ptr<MusicRAII> musicRAII = std::make_shared<MusicRAII>(absolutePath.string());
    musics[absolutePath] = musicRAII;
    return musicRAII;
}

std::shared_ptr<SoundEffectRAII> ResourceCache::LoadSoundEffect(const std::filesystem::path& filePath)
{
    // Get absolute path to the file.
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);

    // Return cached sound effect if it was already loaded.
    if (soundEffects.contains(absolutePath))
        return soundEffects[absolutePath];

    // Load the sound effect and cache it.
    std::shared_ptr<SoundEffectRAII> soundEffectRAII = std::make_shared<SoundEffectRAII>(absolutePath.string());
    soundEffects[absolutePath] = soundEffectRAII;
    return soundEffectRAII;
}

std::shared_ptr<SDLSurfaceRAII> ResourceCache::LoadSurface(const std::filesystem::path& filePath)
{
    // Get absolute path to the file.
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);

    // Return cached surface if it was already loaded.
    if (surfaces.contains(absolutePath))
        return surfaces[absolutePath];

    // Load the surface and cache it.
    std::shared_ptr<SDLSurfaceRAII> surfaceRAII = details::LoadSurfaceWithStreamingAccess(absolutePath);
    surfaces[absolutePath] = surfaceRAII;
    return surfaceRAII;
}

std::shared_ptr<SDLTextureRAII> ResourceCache::GetColoredPixelTexture(const ColorName& color)
{
    // Return cached texture if it was already loaded.
    if (coloredTextures.contains(color))
        return coloredTextures[color];

    // Create texture with the specified color and cache it.
    std::shared_ptr<SDLTextureRAII> textureRAII = std::make_shared<SDLTextureRAII>(details::GetColoredPixelTexture(renderer, color));
    coloredTextures[color] = textureRAII;
    return textureRAII;
}

} // namespace details