#include "sdl_RAII.h"
#include <stdexcept>
#include <utils/logger.h>

SDLInitializerRAII::SDLInitializerRAII(Uint32 flags)
{
    if (SDL_Init(flags) < 0)
    {
        throw std::runtime_error(MY_FMT("SDL could not initialize! SDL_Error: {}", SDL_GetError()));
    }
}

SDLInitializerRAII::~SDLInitializerRAII()
{
    SDL_Quit();
}

SDLWindowRAII::SDLWindowRAII(const std::string& title, int width, int height)
{
    init(title, width, height);
}

SDLWindowRAII::SDLWindowRAII(const std::string& title, glm::vec2 windowSize)
{
    init(title, static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
}

SDLWindowRAII::~SDLWindowRAII()
{
    if (window)
    {
        SDL_DestroyWindow(window);
    }
}

void SDLWindowRAII::init(const std::string& title, int width, int height)
{
    window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window)
    {
        throw std::runtime_error(MY_FMT("Failed to create SDL Window: {}", SDL_GetError()));
    }
}

SDLRendererRAII::SDLRendererRAII(SDL_Window* window, Uint32 flags)
{
    renderer = SDL_CreateRenderer(window, -1, flags);
    if (!renderer)
    {
        throw std::runtime_error(MY_FMT("Failed to create SDL Renderer: {}", SDL_GetError()));
    }
}

SDLRendererRAII::~SDLRendererRAII()
{
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }
}

SDLTextureRAII::SDLTextureRAII(SDL_Texture* texture) : texture(texture)
{
    if (!texture)
        throw std::runtime_error("SDLTextureRAII is nullptr");

    MY_LOG(trace, "SDLTextureRAII created: {}", static_cast<void*>(texture));
}

SDLTextureRAII::~SDLTextureRAII()
{
    if (texture)
    {
        SDL_DestroyTexture(texture);
    }

    MY_LOG(trace, "SDLTextureRAII destroyed: {}", static_cast<void*>(texture));
}

SDLTextureRAII::SDLTextureRAII(SDLTextureRAII&& other) noexcept : texture(std::exchange(other.texture, nullptr))
{}

SDLTextureRAII& SDLTextureRAII::operator=(SDLTextureRAII&& other) noexcept
{
    std::swap(texture, other.texture);
    return *this;
}

SDLTextureLockRAII::SDLTextureLockRAII(SDL_Texture* texture) : texture(texture), pixels(nullptr), pitch(0)
{
    if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0)
    {
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
        throw std::runtime_error("SDL_LockTexture failed");
    }
}

SDLTextureLockRAII::~SDLTextureLockRAII()
{
    if (texture)
    {
        SDL_UnlockTexture(texture);
    }
}

SDLSurfaceLockRAII::SDLSurfaceLockRAII(SDL_Surface* surface) : surface(surface)
{
    if (surface && SDL_MUSTLOCK(surface))
    {
        SDL_LockSurface(surface);
    }
}

SDLSurfaceLockRAII::~SDLSurfaceLockRAII()
{
    if (surface && SDL_MUSTLOCK(surface))
    {
        SDL_UnlockSurface(surface);
    }
}

SDLSurfaceRAII::SDLSurfaceRAII(SDL_Surface* surface) : surface(surface)
{
    if (!this->surface)
    {
        throw std::runtime_error("Null surface provided to SDLSurfaceRAII");
    }
}

SDLSurfaceRAII::~SDLSurfaceRAII()
{
    SDL_FreeSurface(surface);
}

SDLSurfaceRAII::SDLSurfaceRAII(SDLSurfaceRAII&& other) noexcept : surface(std::exchange(other.surface, nullptr))
{}

SDLSurfaceRAII& SDLSurfaceRAII::operator=(SDLSurfaceRAII&& other) noexcept
{
    std::swap(surface, other.surface);
    return *this;
}
SDLPixelFormatRAII::SDLPixelFormatRAII(SDL_PixelFormat* format) : pixelFormat(format)
{
    if (!pixelFormat)
        throw std::runtime_error("[SDLPixelFormatRAII] format is nullptr");
}

SDLPixelFormatRAII::~SDLPixelFormatRAII()
{
    if (pixelFormat)
    {
        SDL_FreeFormat(pixelFormat);
    }
}

SDLPixelFormatRAII::SDLPixelFormatRAII(SDLPixelFormatRAII&& other) noexcept : pixelFormat(std::exchange(other.pixelFormat, nullptr))
{}

SDLPixelFormatRAII& SDLPixelFormatRAII::operator=(SDLPixelFormatRAII&& other) noexcept
{
    std::swap(pixelFormat, other.pixelFormat);
    return *this;
}
