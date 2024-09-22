#pragma once
#include <SDL.h>
#include <entt/entt.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/resources/resource_manager.h>
#include <utils/sdl/sdl_colors.h>
#include <utils/sdl/sdl_primitives_renderer.h>

class RenderWorldSystem
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    ResourceManager& resourceManager;
    GameOptions& gameState;
    CoordinatesTransformer coordinatesTransformer;
    SdlPrimitivesRenderer& primitivesRenderer;
public:
    RenderWorldSystem(entt::registry& registry, SDL_Renderer* renderer, ResourceManager& resourceManager, SdlPrimitivesRenderer& primitivesRenderer);
    void Render();
private: //////////////////////////// Render game objects methods. //////////////////////////
    void RenderBackground();
    void RenderTiles();
    void RenderAnimations();
    void RenderPlayerWeaponDirection();
    void RenderBoudingBoxes();
    void RenderBox2dSensors();
    void RenderDebugVisualObjects();
};
