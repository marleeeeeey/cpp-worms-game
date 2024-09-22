#include "render_world_system.h"
#include "utils/math_utils.h"
#include <SDL_render.h>
#include <ecs/components/animation_components.h>
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/rendering_components.h>
#include <my_cpp_utils/config.h>
#include <utils/box2d/box2d_glm_operators.h>
#include <utils/debug_tools/debug_draw_bounding_box.h>
#include <utils/logger.h>
#include <utils/sdl/sdl_colors.h>

RenderWorldSystem::RenderWorldSystem(entt::registry& registry, SDL_Renderer* renderer, ResourceManager& resourceManager, SdlPrimitivesRenderer& primitivesRenderer)
  : registry(registry), renderer(renderer), resourceManager(resourceManager), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    coordinatesTransformer(registry), primitivesRenderer(primitivesRenderer)
{}

void RenderWorldSystem::Render()
{
    // Clear the screen with white color.
    SetRenderDrawColor(renderer, ColorName::Black);
    SDL_RenderClear(renderer);

    RenderBackground();
    RenderTiles();
    RenderAnimations();
    RenderPlayerWeaponDirection();

    if (utils::GetConfig<bool, "RenderWorldSystem.debugDrawBoundingBoxes">())
        RenderBoudingBoxes();
    if (utils::GetConfig<bool, "RenderWorldSystem.debugDrawBox2dSensors">())
        RenderBox2dSensors();

    RenderDebugVisualObjects();
}

void RenderWorldSystem::RenderBackground()
{
    auto backgroundInfo = gameState.levelOptions.backgroundInfo;
    primitivesRenderer.RenderBackground(backgroundInfo);
}

void RenderWorldSystem::RenderTiles()
{
    for (const auto zOrderingType : magic_enum::enum_values<ZOrderingType>())
    {
        auto tilesView = registry.view<TileComponent, PhysicsComponent>();
        for (auto entity : tilesView)
        {
            const auto& [tileComponent, physicalBody] = tilesView.get<TileComponent, PhysicsComponent>(entity);
            if (tileComponent.zOrderingType != zOrderingType)
                continue;

            const glm::vec2 posWorld = coordinatesTransformer.PhysicsToWorld(physicalBody.bodyRAII->GetBody()->GetPosition());
            const float angle = physicalBody.bodyRAII->GetBody()->GetAngle();
            primitivesRenderer.RenderTile(tileComponent, posWorld, angle);
        }
    }
}

void RenderWorldSystem::RenderPlayerWeaponDirection()
{
    auto players = registry.view<PhysicsComponent, PlayerComponent, AnimationComponent>();
    for (auto entity : players)
    {
        auto [physicalBody, playerInfo, animationComponent] = players.get<PhysicsComponent, PlayerComponent, AnimationComponent>(entity);

        // Draw the weapon.
        // TODO1: Currently we are always get the animation in initial state. So it always draws the first frame.
        // We should use AnimationComponent to make weapon animation runnable.
        const glm::vec2 playerPosWorld = coordinatesTransformer.PhysicsToWorld(physicalBody.bodyRAII->GetBody()->GetPosition());
        float angle = utils::GetAngleFromDirection(playerInfo.weaponDirection);
        auto weaponAnimation = resourceManager.GetAnimation("scepter");
        SDL_RendererFlip weaponFlip = animationComponent.flip == SDL_FLIP_HORIZONTAL ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
        primitivesRenderer.RenderAnimationFirstFrame(weaponAnimation, playerPosWorld, angle, weaponFlip);
    }
}

void RenderWorldSystem::RenderAnimations()
{
    auto view = registry.view<AnimationComponent, PhysicsComponent>();

    for (auto entity : view)
    {
        const auto& [animationInfo, physicsInfo] = view.get<AnimationComponent, PhysicsComponent>(entity);

        // Caclulate the position and angle of the animation.
        auto body = physicsInfo.bodyRAII->GetBody();

        glm::vec2 physicsBodyCenterWorld = coordinatesTransformer.PhysicsToWorld(body->GetPosition());
        const float angle = body->GetAngle();

        primitivesRenderer.RenderAnimationComponent(animationInfo, physicsBodyCenterWorld, angle);

        if (utils::GetConfig<bool, "RenderWorldSystem.debugRenderPlayerHitbox">())
        {
            primitivesRenderer.RenderRect(physicsBodyCenterWorld, animationInfo.GetHitboxSize(), angle, ColorName::Green);
        }
    }
}

void RenderWorldSystem::RenderBoudingBoxes()
{
    auto& pr = primitivesRenderer;
    auto& ct = coordinatesTransformer;
    DrawBoudingBoxes(pr, ct, registry.view<PhysicsComponent, PlayerComponent>(), ColorName::Green);
    DrawBoudingBoxes(pr, ct, registry.view<PhysicsComponent, DestructibleComponent>(), ColorName::Yellow);
}

void RenderWorldSystem::RenderBox2dSensors()
{
    auto& pr = primitivesRenderer;
    auto& ct = coordinatesTransformer;
    DrawSensorBoxes(pr, ct, registry.view<PhysicsComponent>(), ColorName::Red);
}

void RenderWorldSystem::RenderDebugVisualObjects()
{
    auto& pr = primitivesRenderer;
    auto& ct = coordinatesTransformer;
    DrawBoudingBoxes(pr, ct, registry.view<PhysicsComponent, DebugVisualObjectComponent>(), ColorName::Yellow);
}
