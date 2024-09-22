#include "sdl_primitives_renderer.h"
#include <glm/fwd.hpp>
#include <numbers>
#include <utils/sdl/sdl_colors.h>
#include <utils/sdl/sdl_gfx_wrapper.h>
#include <utils/sdl/sdl_utils.h>
#include <vector>

SdlPrimitivesRenderer::SdlPrimitivesRenderer(entt::registry& registry, SDL_Renderer* renderer)
  : renderer(renderer), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), coordinatesTransformer(registry)
{}

void SdlPrimitivesRenderer::RenderRect(const glm::vec2& posWorld, const glm::vec2& sizeWorld, float angle, ColorName color)
{
    auto sdlColor = GetSDLColor(color);
    auto centerPosScreen = coordinatesTransformer.WorldToScreen(posWorld);
    auto sizeScreen = coordinatesTransformer.WorldToScreen(sizeWorld, CoordinatesTransformer::Type::Length);
    auto centerOfRotation = centerPosScreen;

    // Coordinates of the vertices of the rectangle
    std::vector<glm::vec2> vertices = {
        {centerPosScreen.x - sizeScreen.x / 2, centerPosScreen.y - sizeScreen.y / 2},
        {centerPosScreen.x + sizeScreen.x / 2, centerPosScreen.y - sizeScreen.y / 2},
        {centerPosScreen.x + sizeScreen.x / 2, centerPosScreen.y + sizeScreen.y / 2},
        {centerPosScreen.x - sizeScreen.x / 2, centerPosScreen.y + sizeScreen.y / 2}};

    // Rotate the vertices
    for (auto& vertex : vertices)
        utils::RotatePoint(vertex, centerOfRotation, angle);

    DrawPoligon(renderer, vertices, sdlColor);
}

void SdlPrimitivesRenderer::RenderCircle(const glm::vec2& centerWorld, float radiusWorld, ColorName color)
{
    auto centerScreen = coordinatesTransformer.WorldToScreen(centerWorld);
    auto radiusScreen = coordinatesTransformer.WorldToScreen(radiusWorld);
    auto sdlColor = GetSDLColor(color);
    DrawCircle(renderer, centerScreen, radiusScreen, sdlColor);
}

void SdlPrimitivesRenderer::RenderTile(const TileComponent& tileInfo, const glm::vec2& centerWorld, const float angle, const SDL_RendererFlip& flip)
{
    auto sizeWorld = tileInfo.sizeWorld;
    SDL_Rect destRect = GetRectWithCameraTransform(centerWorld, sizeWorld);

    if (!tileInfo.texturePtr)
    {
        RenderRect(centerWorld, sizeWorld, angle, tileInfo.colorName);
        return;
    }

    // Calculate the angle in degrees.
    SDL_Point center = {destRect.w / 2, destRect.h / 2};
    double angleDegrees = angle * 180.0 / std::numbers::pi;

    // Render the tile with the calculated angle.
    SDL_RenderCopyEx(renderer, tileInfo.texturePtr->get(), &tileInfo.textureRect, &destRect, angleDegrees, &center, flip);
}

void SdlPrimitivesRenderer::RenderAnimationComponent(const AnimationComponent& animationInfo, glm::vec2 centerWorld, float angle)
{
    if (animationInfo.animation.frames.empty())
        return;

    // TODO1: Unify using the modulo operator in interface.
    auto safeIndex = animationInfo.currentFrameIndex % animationInfo.animation.frames.size();

    const auto& animation = animationInfo.animation;
    const auto& frame = animation.frames[safeIndex];

    MY_LOG(
        trace, "safeIndex: {}, textureRect: x: {}, y: {}, w: {}, h: {}", safeIndex, frame.tileComponent.textureRect.x, frame.tileComponent.textureRect.y,
        frame.tileComponent.textureRect.w, frame.tileComponent.textureRect.h);

    if (animation.hitboxRect)
    {
        const SDL_Rect& hitboxRect = *animation.hitboxRect;
        auto textureSize = frame.tileComponent.sizeWorld;

        // Shift the center of the animation to the center of the hitbox.
        auto textureCenter = textureSize / 2.0f;
        auto hitboxCenter = glm::vec2(hitboxRect.x, hitboxRect.y) + glm::vec2(hitboxRect.w, hitboxRect.h) / 2.0f;
        auto shift = hitboxCenter - textureCenter;
        auto hitboxNewCenter = centerWorld - shift;
        centerWorld = hitboxNewCenter;
    }

    RenderTile(frame.tileComponent, centerWorld, angle, animationInfo.flip);
}

void SdlPrimitivesRenderer::RenderAnimationFirstFrame(const Animation& animation, glm::vec2 centerWorld, float angle, const SDL_RendererFlip& flip)
{
    if (animation.frames.empty())
        return;

    const auto& frame = animation.frames[0];

    MY_LOG(
        trace, "textureRect: x: {}, y: {}, w: {}, h: {}", frame.tileComponent.textureRect.x, frame.tileComponent.textureRect.y, frame.tileComponent.textureRect.w,
        frame.tileComponent.textureRect.h);

    RenderTile(frame.tileComponent, centerWorld, angle, flip);
}

void SdlPrimitivesRenderer::RenderBackground(const BackgroundInfo& backgroundInfo)
{
    auto textureRAII = backgroundInfo.texture;
    if (!textureRAII)
    {
        MY_LOG(warn, "No background texture to render");
        return;
    }
    auto backgroundTexture = textureRAII->get();

    auto textureScale = backgroundInfo.textureScale;

    int textureWidth, textureHeight;
    SDL_QueryTexture(backgroundTexture, nullptr, nullptr, &textureWidth, &textureHeight);

    // Calculate the size of the texture after scaling.
    textureWidth *= textureScale;
    textureHeight *= textureScale;

    // Calculate the background offset depending on the camera position and the "depth" of the background.
    auto backgroundCenter = gameState.windowOptions.cameraCenterSdl * backgroundInfo.speedFactor;
    auto backgroundTopLeft = backgroundCenter - glm::vec2(textureWidth, textureHeight) / 2.0f;

    // Set the destination rectangle for the texture.
    SDL_Rect dstRect;
    dstRect.x = backgroundTopLeft.x;
    dstRect.y = backgroundTopLeft.y;
    dstRect.w = textureWidth;
    dstRect.h = textureHeight;

    // Render the background texture.
    SDL_RenderCopy(renderer, backgroundTexture, nullptr, &dstRect);
}

//////////////////////// Helper methods ////////////////////////

SDL_Rect SdlPrimitivesRenderer::GetRectWithCameraTransform(const glm::vec2& posWorld, const glm::vec2& sizeWorld)
{
    auto& rOpt = gameState.windowOptions;

    glm::vec2 transformedPosition = (posWorld - rOpt.cameraCenterSdl) * rOpt.cameraScale + rOpt.windowSize / 2.0f;

    // Have to render from the center of the object. Because the Box2D body is in the center of the object.
    SDL_Rect rect = {
        static_cast<int>(transformedPosition.x - sizeWorld.x * rOpt.cameraScale / 2), static_cast<int>(transformedPosition.y - sizeWorld.y * rOpt.cameraScale / 2),
        static_cast<int>(sizeWorld.x * rOpt.cameraScale), static_cast<int>(sizeWorld.y * rOpt.cameraScale)};

    return rect;
}
