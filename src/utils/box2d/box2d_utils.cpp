#include "box2d_utils.h"
#include <ecs/components/physics_components.h>
#include <utils/box2d/box2d_glm_operators.h>
#include <utils/math_utils.h>
#include <utils/vec_operators.h>

Box2dUtils::Box2dUtils(entt::registry& registry)
  : registry(registry), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), box2dBodyCreator(registry), coordinatesTransformer(registry)
{}

void Box2dUtils::ApplyForceToPhysicalBodies(std::vector<entt::entity> physicalEntities, const glm::vec2& forceCenterWorld, float force)
{
    auto forceCenterPhysics = coordinatesTransformer.WorldToPhysics(forceCenterWorld);

    for (auto& entity : physicalEntities)
    {
        auto originalObjPhysicsInfo = registry.get<PhysicsComponent>(entity).bodyRAII->GetBody();
        const b2Vec2& posPhysics = originalObjPhysicsInfo->GetPosition();

        // Make target body as dynamic.
        originalObjPhysicsInfo->SetType(b2_dynamicBody);

        // Apply force to the target.
        // Force direction is from grenade to target. Inside. This greate interesting effect.
        auto forceVec = -(posPhysics - forceCenterPhysics) * force;
        originalObjPhysicsInfo->ApplyForceToCenter(forceVec, true);
    }
}
