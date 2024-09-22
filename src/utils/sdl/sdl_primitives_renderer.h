#pragma once
#include "utils/animation.h"
#include <SDL.h>
#include <ecs/components/animation_components.h>
#include <ecs/components/rendering_components.h>
#include <entt/entt.hpp>
#include <utils/box2d/box2d_RAII.h>
#include <utils/coordinates_transformer.h>
#include <utils/game_options.h>
#include <utils/resources/resource_manager.h>
#include <utils/sdl/sdl_colors.h>

class SdlPrimitivesRenderer
{
    SDL_Renderer* renderer;
    GameOptions& gameState;
    CoordinatesTransformer coordinatesTransformer;
public:
    SdlPrimitivesRenderer(entt::registry& registry, SDL_Renderer* renderer);
public:
    void RenderRect(const glm::vec2& posWorld, const glm::vec2& sizeWorld, float angle, ColorName color);
    void RenderCircle(const glm::vec2& centerWorld, float radiusWorld, ColorName color);
    void RenderTile(const TileComponent& tileInfo, const glm::vec2& centerWorld, const float angle, const SDL_RendererFlip& flip = SDL_FLIP_NONE);
    void RenderAnimationComponent(const AnimationComponent& animationInfo, glm::vec2 centerWorld, float angle);
    void RenderAnimationFirstFrame(const Animation& animation, glm::vec2 centerWorld, float angle, const SDL_RendererFlip& flip = SDL_FLIP_NONE);
    void RenderBackground(const BackgroundInfo& backgroundInfo);
private: // Helper methods.
    SDL_Rect GetRectWithCameraTransform(const glm::vec2& posWorld, const glm::vec2& sizeWorld);
};