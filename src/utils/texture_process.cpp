#include "texture_process.h"
#include "SDL_pixels.h"
#include "SDL_surface.h"
#include <SDL_image.h>
#include <my_common_cpp_utils/logger.h>
#include <utils/sdl_RAII.h>

bool IsTileInvisible(SDL_Surface* surface, const SDL_Rect& miniTextureSrcRect)
{
    if (!surface)
        throw std::runtime_error("[IsTileInvisible] Surface is NULL");

    SDLSurfaceLockRAII lock(surface);

    Uint32* pixels = static_cast<Uint32*>(surface->pixels);
    int pitch = surface->pitch;

    for (int row = 0; row < miniTextureSrcRect.h; ++row)
    {
        for (int col = 0; col < miniTextureSrcRect.w; ++col)
        {
            Uint32 pixel = pixels[(miniTextureSrcRect.y + row) * (pitch / 4) + (miniTextureSrcRect.x + col)];
            Uint8 alpha = (pixel >> surface->format->Ashift) & 0xFF;
            if (alpha > 0)
            {
                return false;
            }
        }
    }

    return true;
}

SDL_Rect CalculateSrcRect(int tileId, int tileWidth, int tileHeight, std::shared_ptr<SDLTextureRAII> texture)
{
    int textureWidth, textureHeight;
    SDL_QueryTexture(texture->get(), nullptr, nullptr, &textureWidth, &textureHeight);

    int tilesPerRow = textureWidth / tileWidth;
    tileId -= 1; // Adjust tileId to match 0-based indexing. Tiled uses 1-based indexing.

    SDL_Rect srcRect;
    srcRect.x = (tileId % tilesPerRow) * tileWidth;
    srcRect.y = (tileId / tilesPerRow) * tileHeight;
    srcRect.w = tileWidth;
    srcRect.h = tileHeight;

    return srcRect;
}

std::vector<SDL_Rect> SplitRect(const SDL_Rect& rect, int m, int n)
{
    std::vector<SDL_Rect> result;
    int width = rect.w / m;
    int height = rect.h / n;

    for (int y = 0; y < n; ++y)
    {
        for (int x = 0; x < m; ++x)
        {
            SDL_Rect small_rect = {rect.x + x * width, rect.y + y * height, width, height};
            result.push_back(small_rect);
        }
    }

    return result;
}

std::vector<SDL_Rect> DivideRectByCellSize(const SDL_Rect& rect, const SDL_Point& cellSize)
{
    std::vector<SDL_Rect> cells;

    // Calculate the number of cells horizontally and vertically
    int horizontalCells = rect.w / cellSize.x;
    int verticalCells = rect.h / cellSize.y;

    // Loop through each cell and create a rectangle for it
    for (int y = 0; y < verticalCells; ++y)
    {
        for (int x = 0; x < horizontalCells; ++x)
        {
            SDL_Rect cellRect = {rect.x + x * cellSize.x, rect.y + y * cellSize.y, cellSize.x, cellSize.y};
            cells.push_back(cellRect);
        }
    }

    return cells;
}

namespace details
{

std::shared_ptr<SDLTextureRAII> LoadTexture(SDL_Renderer* renderer, const std::filesystem::path& imagePath)
{
    SDL_Texture* texture = IMG_LoadTexture(renderer, imagePath.string().c_str());

    if (texture == nullptr)
        throw std::runtime_error(MY_FMT("Failed to load texture: {}", imagePath.string()));

    return std::make_shared<SDLTextureRAII>(texture);
}

SDL_Surface* ConvertSurfaceFormat(SDL_Surface* srcSurface, Uint32 toFormatEnum)
{
    if (!srcSurface)
        throw std::runtime_error("[ConvertSurfaceFormat] Source surface is NULL");

    SDLPixelFormatRAII toFormat = SDL_AllocFormat(toFormatEnum);

    // Create a new surface with the desired format.
    SDL_Surface* convertedSurface = SDL_ConvertSurface(srcSurface, toFormat.get(), 0);
    if (!convertedSurface)
        throw std::runtime_error(MY_FMT("[ConvertSurfaceFormat] Failed to convert surface: {}", SDL_GetError()));

    return convertedSurface;
}

std::shared_ptr<SDLSurfaceRAII> LoadSurfaceWithStreamingAccess(
    SDL_Renderer* renderer, const std::filesystem::path& imagePath)
{
    std::string imagePathStr = imagePath.string();

    // Step 1. Load image into SDL_Surface.
    SDLSurfaceRAII surface = IMG_Load(imagePathStr.c_str());

    // Convert surface to target format if necessary.
    auto targetFormat = SDL_PIXELFORMAT_ABGR8888;
    if (surface.get()->format->format != targetFormat)
    {
        MY_LOG_FMT(warn, "Original surface format: {}", SDL_GetPixelFormatName(surface.get()->format->format));
        surface = ConvertSurfaceFormat(surface.get(), targetFormat);
        MY_LOG_FMT(warn, "Converted surface format: {}", SDL_GetPixelFormatName(surface.get()->format->format));
    }
    else
    {
        MY_LOG_FMT(info, "Surface format: {}", SDL_GetPixelFormatName(surface.get()->format->format));
    }

    return std::make_shared<SDLSurfaceRAII>(std::move(surface));
}

} // namespace details
