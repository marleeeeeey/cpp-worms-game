#include "portals_game_logic_system.h"
#include <ecs/components/event_components.h>
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/portal_components.h>
#include <entt/entity/fwd.hpp>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/math_utils.h>
#include <utils/box2d/box2d_glm_operators.h>
#include <utils/entt/entt_registry_requests.h>
#include <utils/logger.h>
#include <utils/systems/audio_system.h>
#include <utils/vec_operators.h>

PortalsGameLogicSystem::PortalsGameLogicSystem(entt::registry& registry, GameObjectsFactory& gameObjectsFactory, AudioSystem& audioSystem)
  : registry(registry), registryWrapper(registry), bodyTuner(registry), gameObjectsFactory(gameObjectsFactory), coordinatesTransformer(registry),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), audioSystem(audioSystem)
{}

void PortalsGameLogicSystem::Update(float deltaTime)
{
    bool enabled = utils::GetConfig<bool, "PortalsGameLogicSystem.enabled">();
    if (!enabled)
        return;

    UpdatePortalsPosition(deltaTime);
    MagnetFoodToPortal(deltaTime);
    DestroyClosestFoodInPortal();
    ScatterPortalsIsTheyCloseToEachOther();
    EatThePlayerByPortalIfCloser();
    CheckGameCompletness();
}

void PortalsGameLogicSystem::UpdatePortalsPosition(float deltaTime)
{
    if (deltaTime == 0.0f)
        return;

    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    for (auto portalEntity : portalEntities)
    {
        auto& portalComponent = portalEntities.get<PortalComponent>(portalEntity);
        if (portalComponent.isSleeping)
            continue;

        UpdatePortalTarget(portalEntity);
        if (!portalComponent.target)
            continue;

        // Apply the force to phiysics body to move it to the closest target
        auto& physicsComponent = portalEntities.get<PhysicsComponent>(portalEntity);
        auto portalBody = physicsComponent.bodyRAII->GetBody();
        auto portalPos = portalBody->GetPosition();
        b2Vec2 direction = portalComponent.target->second - portalPos;
        direction.Normalize();
        // change object speed to the target speed
        b2Vec2 speed = portalBody->GetLinearVelocity();
        b2Vec2 targetSpeed = direction * portalComponent.speed;
        b2Vec2 speedDiff = targetSpeed - speed;
        b2Vec2 force = speedDiff * portalBody->GetMass() / deltaTime;
        portalBody->ApplyForceToCenter(force, true);
    }
}

void PortalsGameLogicSystem::UpdatePortalTarget(entt::entity portalEntity)
{
    if (utils::GetConfig<bool, "PortalsGameLogicSystem.debugOnlyNoTargetForPortal">())
        return;

    auto& portal = registry.get<PortalComponent>(portalEntity);
    auto& physicsComponent = registry.get<PhysicsComponent>(portalEntity);
    auto portalBody = physicsComponent.bodyRAII->GetBody();
    auto portalPos = portalBody->GetPosition();

    std::optional<std::pair<PortalComponent::PortalTargetType, b2Vec2>> newTarget;

    auto closestExplosionParticlesPos = request::FindClosestEntityPosWithAllComponents<ExplostionParticlesComponent>(registry, portalPos);
    if (closestExplosionParticlesPos.has_value())
    {
        // If the closest explosion particles are too close to the portal, return this position.
        if (b2Distance(closestExplosionParticlesPos.value(), portalPos) < 3.0f)
            newTarget = std::make_pair(PortalComponent::PortalTargetType::DestructibleParticle, closestExplosionParticlesPos.value());
    }

    auto closestStickyPos = request::FindClosestEntityPosWithAllComponents<StickyComponent>(registry, portalPos);
    if (!newTarget && closestStickyPos.has_value())
        portal.target = std::make_pair(PortalComponent::PortalTargetType::DestructibleParticle, closestStickyPos.value());

    auto closestPlayer = request::FindClosestEntityWithAllComponents<PlayerComponent>(registry, portalPos);
    if (!newTarget && closestPlayer.has_value())
    {
        const auto& playerPos = registry.get<PhysicsComponent>(closestPlayer.value()).bodyRAII->GetBody()->GetPosition();
        newTarget = std::make_pair(PortalComponent::PortalTargetType::Player, playerPos);
    }

    // Play the sound effect if the target is changed to the player.
    if (newTarget && newTarget->first == PortalComponent::PortalTargetType::Player)
    {
        if ((portal.target && portal.target->first != PortalComponent::PortalTargetType::Player) || !portal.target)
            audioSystem.PlaySoundEffect("portal_go_to_player");
    }

    portal.target = newTarget;
}

void PortalsGameLogicSystem::MagnetFoodToPortal(float deltaTime)
{
    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    for (auto entity : portalEntities)
    {
        auto& portalComponent = portalEntities.get<PortalComponent>(entity);
        if (portalComponent.isSleeping)
            continue;

        auto& physicsComponent = portalEntities.get<PhysicsComponent>(entity);
        auto portalBody = physicsComponent.bodyRAII->GetBody();
        auto portalPos = portalBody->GetPosition();

        auto entitiesToMagnet = request::FindEntitiesWithAllComponentsInRadius<ExplostionParticlesComponent>(registry, portalPos, 6.0f);

        for (auto entityToMagnet : entitiesToMagnet)
        {
            auto& physicsComponentToMagnet = registry.get<PhysicsComponent>(entityToMagnet);
            auto bodyToMagnet = physicsComponentToMagnet.bodyRAII->GetBody();
            auto bodyPos = bodyToMagnet->GetPosition();
            b2Vec2 direction = portalPos - bodyPos;
            direction.Normalize();
            float distance = b2Distance(portalPos, bodyPos);
            float force = portalComponent.magnetForce * (1.0f / sqrt(distance));
            bodyToMagnet->ApplyForceToCenter(force * direction * deltaTime, true);
        }
    }
}

void PortalsGameLogicSystem::DestroyClosestFoodInPortal()
{
    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();

    std::optional<entt::entity> portalToDestroyOpt;

    portalEntities.each(
        [this, &portalToDestroyOpt](entt::entity portalEntity, PhysicsComponent& portalPhysics, PortalComponent& portalComponent)
        {
            if (portalComponent.isSleeping)
                return;

            auto portalBody = portalPhysics.bodyRAII->GetBody();
            auto portalPos = portalBody->GetPosition();

            auto entitiesInPortal = request::FindEntitiesWithAllComponentsInRadius<ExplostionParticlesComponent>(registry, portalPos, 0.1f);
            if (entitiesInPortal.empty())
                return;

            for (auto entityInPortal : entitiesInPortal)
            {
                portalComponent.foodCounter++;

                const auto& portalMaxFoodCounter = utils::GetConfig<size_t, "PortalsGameLogicSystem.portalMaxFoodCounter">();
                if (portalComponent.foodCounter >= portalMaxFoodCounter)
                {
                    auto portalPosWorld = coordinatesTransformer.PhysicsToWorld(portalPos);
                    gameObjectsFactory.SpawnPlayer(portalPosWorld, "Rescued player");
                    portalToDestroyOpt = portalEntity;
                    return;
                }

                registryWrapper.Destroy(entityInPortal);
            }

            audioSystem.PlaySoundEffect("portal_feeds");
        });

    if (portalToDestroyOpt.has_value())
    {
        // Reset the food counter for all portals.
        portalEntities.each([](entt::entity, PhysicsComponent&, PortalComponent& portalComponent) { portalComponent.foodCounter = 0; });
        // Destroy one portal.
        registryWrapper.Destroy(portalToDestroyOpt.value());
    }
}

void PortalsGameLogicSystem::ScatterPortalsIsTheyCloseToEachOther()
{
    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    for (auto entity : portalEntities)
    {
        auto& physicsComponent = portalEntities.get<PhysicsComponent>(entity);
        auto portalBody = physicsComponent.bodyRAII->GetBody();
        auto portalPos = portalBody->GetPosition();

        auto mergedPortals = request::FindEntitiesWithAllComponentsInRadius<PortalComponent>(registry, portalPos, 0.5f);
        // If there are more than one portal in the same position, scatter them.
        if (mergedPortals.size() > 1)
        {
            // Caclulate the center of the merged portals.
            b2Vec2 mergePortalCenterPos = b2Vec2_zero;
            for (auto mergedPortal : mergedPortals)
            {
                auto& mergedPortalPhysicsComponent = registry.get<PhysicsComponent>(mergedPortal);
                mergePortalCenterPos += mergedPortalPhysicsComponent.bodyRAII->GetBody()->GetPosition();
            }
            mergePortalCenterPos *= 1.0f / mergedPortals.size();

            // Apply the force to phiysics bodies to scatter them.
            for (auto portal : mergedPortals)
            {
                auto& mergedPortalPhysicsComponent = registry.get<PhysicsComponent>(portal);
                auto mergedPortalBody = mergedPortalPhysicsComponent.bodyRAII->GetBody();
                b2Vec2 direction = mergedPortalBody->GetPosition() - mergePortalCenterPos;
                direction.Normalize();
                mergedPortalBody->ApplyForceToCenter(500.0f * direction, true);

                // Sleep the portal for a while.
                auto& portalComponent = registry.get<PortalComponent>(portal);
                portalComponent.isSleeping = true;
                registry.emplace_or_replace<TimeEventComponent>(
                    portal, utils::Random<float>(0.2f, 0.5f),
                    [this](entt::entity portalEntity)
                    {
                        auto& portalComponent = registry.get<PortalComponent>(portalEntity);
                        portalComponent.isSleeping = false;
                    });
            }
        }
    }
}

void PortalsGameLogicSystem::EatThePlayerByPortalIfCloser()
{
    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    portalEntities.each(
        [this](auto portalEntity, auto& physicsComponent, auto& portalComponent)
        {
            if (portalComponent.isSleeping)
                return;

            auto portalBody = physicsComponent.bodyRAII->GetBody();
            auto portalPos = portalBody->GetPosition();

            auto playerEntityOpt = request::FindClosestEntityWithAllComponents<PlayerComponent>(registry, portalPos);

            if (!playerEntityOpt.has_value())
                return;

            auto playerEntity = playerEntityOpt.value();

            auto& playerPhysicsComponent = registry.get<PhysicsComponent>(playerEntity);
            auto playerBody = playerPhysicsComponent.bodyRAII->GetBody();
            auto playerBodyPos = playerBody->GetPosition();

            auto portalEatPlayerWithDistance = utils::GetConfig<float, "PortalsGameLogicSystem.portalEatPlayerWithDistance">();
            if (b2Distance(portalPos, playerBodyPos) < portalEatPlayerWithDistance)
            {
                MY_LOG(debug, "Player {} is eaten by the portal {}!", playerEntity, portalEntity);
                registryWrapper.Destroy(playerEntity);

                // Spawn a new portal near thisw place.
                auto portalPosWorld = coordinatesTransformer.PhysicsToWorld(portalPos);
                gameObjectsFactory.SpawnPortal(portalPosWorld, "Respawned portal");
            }
        });
}

void PortalsGameLogicSystem::CheckGameCompletness()
{
    auto playerEntities = registry.view<PlayerComponent>();
    if (playerEntities.size() == 0)
    {
        gameState.controlOptions.showGameOverScreen = true;
    }

    auto portalEntities = registry.view<PortalComponent>();
    if (portalEntities.size() == 0)
    {
        gameState.controlOptions.showLevelCompleteScreen = true;
    }
}
