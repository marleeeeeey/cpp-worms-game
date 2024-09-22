#include "turret_game_logic_system.h"
#include "ecs/components/animation_components.h"
#include "utils/math_utils.h"
#include <ecs/components/physics_components.h>
#include <ecs/components/turret_component.h>
#include <ecs/components/weapon_components.h>
#include <entt/entt.hpp>
#include <my_cpp_utils/math_utils.h>

TurretGameLogicSystem::TurretGameLogicSystem(entt::registry& registry, GameObjectsFactory& gameObjectsFactory, CoordinatesTransformer& coordinatesTransformer)
  : registry(registry), gameObjectsFactory(gameObjectsFactory), coordinatesTransformer(coordinatesTransformer)
{}

void TurretGameLogicSystem::Update()
{
    registry.view<TurretComponent, PhysicsComponent, FireRateComponent, AnimationComponent>().each(
        [this]([[maybe_unused]] entt::entity entity, TurretComponent& turret, PhysicsComponent& physics, FireRateComponent& fireRate, AnimationComponent& animation)
        {
            if (!turret.shooting)
                return;

            if (fireRate.remainingFireRate > 0.0f)
                return;

            glm::vec2 initialBulletPosWorld = coordinatesTransformer.PhysicsToWorld(physics.bodyRAII->GetBody()->GetPosition());

            float bodyAngle = physics.bodyRAII->GetBody()->GetAngle();
            float gunGirection = utils::GetAngleFromDirection(turret.gunGirection);
            gunGirection += utils::Random<float>(-0.5f, 0.5f);
            float bulletAngle = bodyAngle + gunGirection;
            initialBulletPosWorld += utils::GetDirectionFromAngle<glm::vec2>(bulletAngle) * animation.GetHitboxSize().x / 2.0f;

            float initialBulletSpeed = utils::Random<float>(4.0f, 7.0f);
            gameObjectsFactory.SpawnBullet(initialBulletPosWorld, initialBulletSpeed, bulletAngle, turret.weaponProps);
        });
}
