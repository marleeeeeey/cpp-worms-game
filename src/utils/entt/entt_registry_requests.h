#pragma once
#include <Box2D/Box2D.h>
#include <ecs/components/physics_components.h>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <optional>

namespace request
{

template <typename... ComponentTypes>
std::optional<entt::entity> FindClosestEntityWithAllComponents(entt::registry& registry, const b2Vec2& anchorPosWorld, std::function<bool(entt::entity)> optTruePredicate = nullptr)
{
    auto targetEntities = registry.view<PhysicsComponent, ComponentTypes...>();
    float minDistance = std::numeric_limits<float>::max();
    std::optional<entt::entity> closestTargetEntity;
    for (auto targetEntity : targetEntities)
    {
        if (optTruePredicate && !optTruePredicate(targetEntity))
            continue;

        auto& targetPhysicsComponent = targetEntities.template get<PhysicsComponent>(targetEntity);
        auto targetPos = targetPhysicsComponent.bodyRAII->GetBody()->GetPosition();
        float distance = b2Distance(targetPos, anchorPosWorld);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestTargetEntity = targetEntity;
        }
    }
    return closestTargetEntity;
}

template <typename... ComponentTypes>
std::optional<b2Vec2> FindClosestEntityPosWithAllComponents(entt::registry& registry, const b2Vec2& anchorPosWorld)
{
    auto entityOpt = FindClosestEntityWithAllComponents<ComponentTypes...>(registry, anchorPosWorld);

    if (!entityOpt.has_value())
        return std::nullopt;

    auto& physicsComponent = registry.get<PhysicsComponent>(entityOpt.value());
    return physicsComponent.bodyRAII->GetBody()->GetPosition();
}

template <typename... ComponentTypes>
std::vector<entt::entity> FindEntitiesWithAllComponentsInRadius(entt::registry& registry, const b2Vec2& centerPosPhysics, float radiusPhysics)
{
    std::vector<entt::entity> entitiesInRadius;
    auto view = registry.view<PhysicsComponent, ComponentTypes...>();

    for (auto entity : view)
    {
        const auto& physicsComponent = view.template get<PhysicsComponent>(entity);
        const auto& entityPos = physicsComponent.bodyRAII->GetBody()->GetPosition();

        if (b2Distance(centerPosPhysics, entityPos) < radiusPhysics)
        {
            entitiesInRadius.push_back(entity);
        }
    }
    return entitiesInRadius;
}

template <typename... ComponentTypes>
std::vector<entt::entity> FilterEntitiesWithAllComponentsInRadius(
    entt::registry& registry, const std::vector<entt::entity>& entities, const b2Vec2& centerPosPhysics, float radiusPhysics)
{
    std::vector<entt::entity> entitiesInRadius;
    for (auto entity : entities)
    {
        // Check if the entity has all the required components.
        if (!registry.all_of<PhysicsComponent, ComponentTypes...>(entity))
            continue;

        auto& physicsComponent = registry.get<PhysicsComponent>(entity);
        auto entityPos = physicsComponent.bodyRAII->GetBody()->GetPosition();
        if (b2Distance(centerPosPhysics, entityPos) < radiusPhysics)
            entitiesInRadius.push_back(entity);
    }
    return entitiesInRadius;
}

template <typename... ComponentTypes>
std::vector<entt::entity> RemoveEntitiesWithAllComponents(entt::registry& registry, const std::vector<entt::entity>& entities)
{
    std::vector<entt::entity> result;
    for (auto& entity : entities)
    {
        if (!registry.all_of<ComponentTypes...>(entity))
            result.push_back(entity);
    }
    return result;
}

template <typename... ComponentTypes>
std::vector<entt::entity> GetEntitiesWithAllComponents(entt::registry& registry, const std::vector<entt::entity>& entities)
{
    std::vector<entt::entity> result;
    for (auto& entity : entities)
    {
        if (registry.all_of<ComponentTypes...>(entity))
            result.push_back(entity);
    }
    return result;
}
} // namespace request