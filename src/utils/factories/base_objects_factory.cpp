#include "base_objects_factory.h"
#include "utils/math_utils.h"
#include <ecs/components/animation_components.h>
#include <ecs/components/event_components.h>
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/portal_components.h>
#include <ecs/components/rendering_components.h>
#include <ecs/components/turret_component.h>
#include <ecs/components/weapon_components.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/math_utils.h>
#include <utils/box2d/box2d_body_options.h>
#include <utils/box2d/box2d_body_tuner.h>
#include <utils/box2d/box2d_utils.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/factories/weapon_props_factory.h>
#include <utils/logger.h>
#include <utils/sdl/sdl_texture_process.h>
#include <utils/sdl/sdl_utils.h>
#include <utils/time_utils.h>

BaseObjectsFactory::BaseObjectsFactory(EnttRegistryWrapper& registryWrapper, ComponentsFactory& componentsFactory)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    box2dBodyCreator(registry), coordinatesTransformer(registry), bodyTuner(registry), componentsFactory(componentsFactory)
{}

entt::entity BaseObjectsFactory::SpawnTile(glm::vec2 posWorld, float sizeWorld, const TextureRect& textureRect, SpawnTileOption tileOptions, const std::string& name)
{
    auto& gap = utils::GetConfig<float, "ObjectsFactory.gapBetweenPhysicalAndVisual">();
    glm::vec2 bodySizeWorld(sizeWorld - gap, sizeWorld - gap);

    auto entity = registryWrapper.Create(name);
    registry.emplace<TileComponent>(entity, glm::vec2(sizeWorld, sizeWorld), textureRect.texture, textureRect.rect, tileOptions.zOrderingType);

    Box2dBodyOptions options;
    options.fixture.restitution = 0.05f;
    switch (tileOptions.destructibleOption)
    {
    case SpawnTileOption::DesctructibleOption::Destructible:
        options.anglePolicy = Box2dBodyOptions::AnglePolicy::Dynamic;
        registry.emplace<DestructibleComponent>(entity);
        break;
    case SpawnTileOption::DesctructibleOption::Indestructible:
        options.dynamic = Box2dBodyOptions::MovementPolicy::Manual;
        options.anglePolicy = Box2dBodyOptions::AnglePolicy::Fixed;
        registry.emplace<IndestructibleComponent>(entity);
        break;
    }

    switch (tileOptions.collidableOption)
    {
    case SpawnTileOption::CollidableOption::Collidable:
        // Default collision flags for options.collisionPolicy
        registry.emplace<CollidableComponent>(entity);
        break;
    case SpawnTileOption::CollidableOption::Transparent:
        options.collisionPolicy = {CollisionFlags::None, CollisionFlags::None};
        registry.emplace<TransparentComponent>(entity);
        break;
    }

    float angle = 0.0f;
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, bodySizeWorld, angle, options);

    return entity;
}

entt::entity BaseObjectsFactory::SpawnFragmentAfterExplosion(const glm::vec2& posWorld)
{
    AnimationComponent fragmentAnimation = componentsFactory.CreateAnimationComponent("explosionFragments", "Fragment[\\d]+", ResourceManager::TagProps::RandomByRegex);
    glm::vec2 fragmentSizeWorld = fragmentAnimation.GetHitboxSize();

    auto entity = registryWrapper.Create("ExplosionFragment");
    registry.emplace<AnimationComponent>(entity, fragmentAnimation);
    float angle = utils::Random<float>(0, 2 * M_PI);
    Box2dBodyOptions options;
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, fragmentSizeWorld, angle, options);
    return entity;
}

entt::entity BaseObjectsFactory::SpawnFlyingEntity(
    const glm::vec2& posWorld, const glm::vec2& sizeWorld, float forceDirection, float initialSpeed, Box2dBodyOptions::AnglePolicy anglePolicy)
{
    // Create the flying entity.
    auto flyingEntity = registryWrapper.Create("flyingEntity");

    // Create a Box2D body for the flying entity.
    Box2dBodyOptions options;
    options.dynamic = Box2dBodyOptions::MovementPolicy::Box2dPhysics;
    options.shape = Box2dBodyOptions::Shape::Box;
    options.anglePolicy = anglePolicy;
    auto& physicsBody = box2dBodyCreator.CreatePhysicsBody(flyingEntity, posWorld, sizeWorld, forceDirection, options);

    // Apply the force to the flying entity.
    b2Vec2 speedVec = b2Vec2(initialSpeed, 0);
    speedVec = b2Mul(b2Rot(forceDirection), speedVec);
    physicsBody.bodyRAII->GetBody()->SetLinearVelocity(speedVec);

    return flyingEntity;
}

entt::entity BaseObjectsFactory::SpawnDebugVisualObject(
    const glm::vec2& posWorld, const glm::vec2& sizeWorld, float angle, const std::string& nameAsKey, const DebugSpawnOptions& debugSpawnOptions)
{
    // Search for entity with the same name component.
    auto namedComponents = registry.view<NameComponent, CreationTimeComponent>();
    std::vector<entt::entity> sameNameEntities = {namedComponents.begin(), namedComponents.end()};
    sameNameEntities.erase(
        std::remove_if(sameNameEntities.begin(), sameNameEntities.end(), [nameAsKey, this](entt::entity e) { return registry.get<NameComponent>(e).name != nameAsKey; }),
        sameNameEntities.end());

    // Check if the entity with the same name already exists.
    if (debugSpawnOptions.spawnPolicy == BaseObjectsFactory::SpawnPolicyBase::This && sameNameEntities.size() > 0)
    {
        MY_LOG(info, "[SpawnDebugVisualObject] Entity with the same name already exists. Entity: {}, SpawnPolicy: {}", nameAsKey, debugSpawnOptions.spawnPolicy);
        return entt::null;
    }

    // Destroy some entities with the same name if the limit is reached.
    if (debugSpawnOptions.spawnPolicy != BaseObjectsFactory::SpawnPolicyBase::All)
    {
        // Sort components by creation time.
        std::sort(
            sameNameEntities.begin(), sameNameEntities.end(),
            [this](entt::entity a, entt::entity b) { return registry.get<CreationTimeComponent>(a).creationTime > registry.get<CreationTimeComponent>(b).creationTime; });

        auto trailSize = debugSpawnOptions.trailSize;

        if (debugSpawnOptions.spawnPolicy == BaseObjectsFactory::SpawnPolicyBase::First)
        {
            // Not nessesary to create new entity. Limit is reached.
            if (sameNameEntities.size() > trailSize)
                return entt::null;

            std::reverse(sameNameEntities.begin(), sameNameEntities.end());
        }

        if (sameNameEntities.size() > trailSize)
        {
            for (size_t i = trailSize; i < sameNameEntities.size(); ++i)
            {
                auto itemToRemove = sameNameEntities[i];
                auto creationTime = registry.get<CreationTimeComponent>(itemToRemove).creationTime;
                MY_LOG(trace, "[SpawnDebugVisualObject] Destroying entity with the same name. Entity: {}, CreationTime: {}", nameAsKey, creationTime);
                registryWrapper.Destroy(itemToRemove);
            }
            sameNameEntities.resize(trailSize);
        }
    }

    auto entity = registryWrapper.Create(nameAsKey);
    registry.emplace<DebugVisualObjectComponent>(entity);
    registry.emplace<NameComponent>(entity, nameAsKey);
    registry.emplace<CreationTimeComponent>(entity, utils::CurrentTimeMilliseconds());

    // PhysicsInfo.
    Box2dBodyOptions options;
    options.shape = Box2dBodyOptions::Shape::Box;
    options.dynamic = Box2dBodyOptions::MovementPolicy::Manual;
    options.anglePolicy = Box2dBodyOptions::AnglePolicy::Fixed;
    options.collisionPolicy = {CollisionFlags::None, CollisionFlags::None};
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, sizeWorld, angle, options);

    MY_LOG(
        trace, "[SpawnDebugVisualObject] Created new DebugVisualObjectComponent: Name {}, Policy {}, Size {}", nameAsKey, debugSpawnOptions.spawnPolicy, sameNameEntities.size());
    return entity;
}

entt::entity BaseObjectsFactory::SpawnDebugVisualObject(entt::entity entity, const std::string& nameAsKey, const DebugSpawnOptions& debugSpawnOptions)
{
    auto& physicsComponent = registry.get<PhysicsComponent>(entity);
    auto body = physicsComponent.bodyRAII->GetBody();
    auto bodyPosPhysics = body->GetPosition();
    auto bodyAnglePhysics = body->GetAngle();
    auto sizeWorld = physicsComponent.options.hitbox.sizeWorld;

    auto posWorld = coordinatesTransformer.PhysicsToWorld(bodyPosPhysics);
    auto newEntity = SpawnDebugVisualObject(posWorld, sizeWorld, bodyAnglePhysics, nameAsKey, debugSpawnOptions);
    return newEntity;
}

std::vector<entt::entity> BaseObjectsFactory::SpawnFragmentsAfterExplosion(glm::vec2 centerWorld, float radiusWorld)
{
    size_t fragmentsCount = static_cast<size_t>(radiusWorld * 0.2f * utils::Random<float>(1, 1.2));
    std::vector<entt::entity> fragments;
    for (size_t i = 0; i < fragmentsCount; ++i)
    {
        auto fragmentRandomPosWorld = utils::GetRandomCoordinateAround(centerWorld, radiusWorld);
        auto fragmentEntity = SpawnFragmentAfterExplosion(fragmentRandomPosWorld);
        fragments.push_back(fragmentEntity);
    }
    Box2dUtils physicsMethods(registry);
    physicsMethods.ApplyForceToPhysicalBodies(fragments, centerWorld, 500.0f);

    return fragments;
}

std::vector<entt::entity> BaseObjectsFactory::SpawnSplittedPhysicalEnteties(const std::vector<entt::entity>& physicalEntities, SDL_Point cellSizeWorld)
{
    assert(cellSizeWorld.x == cellSizeWorld.y);

    std::vector<entt::entity> splittedEntities;

    for (auto& entity : physicalEntities)
    {
        if (!registry.all_of<PhysicsComponent, TileComponent>(entity))
            continue;

        auto originalObjPhysicsInfo = registry.get<PhysicsComponent>(entity).bodyRAII->GetBody();
        auto& originalObjRenderingInfo = registry.get<TileComponent>(entity);
        const b2Vec2& posPhysics = originalObjPhysicsInfo->GetPosition();
        const glm::vec2 originalObjCenterWorld = coordinatesTransformer.PhysicsToWorld(posPhysics);
        const SDL_Rect& originalTextureRect = originalObjRenderingInfo.textureRect;

        // Check if the original object is big enough to be splitted.
        if (originalTextureRect.w <= cellSizeWorld.x || originalTextureRect.h <= cellSizeWorld.y)
            continue;

        auto originalRectCenterInTexture = utils::GetCenterOfRect(originalTextureRect);

        auto pixelTextureRects = utils::DivideRectByCellSize(originalTextureRect, cellSizeWorld);
        for (auto& pixelTextureRect : pixelTextureRects)
        {
            auto pixelRectCenterInTexture = utils::GetCenterOfRect(pixelTextureRect);
            auto pixelCenterShift = pixelRectCenterInTexture - originalRectCenterInTexture;
            glm::vec2 pixelCenterWorld = originalObjCenterWorld + pixelCenterShift;

            SpawnTileOption spawnTileOptions;
            spawnTileOptions.destructibleOption = SpawnTileOption::DesctructibleOption::Destructible;
            spawnTileOptions.zOrderingType = ZOrderingType::Terrain;

            auto pixelEntity = SpawnTile(pixelCenterWorld, cellSizeWorld.x, TextureRect{originalObjRenderingInfo.texturePtr, pixelTextureRect}, spawnTileOptions, "PixeledTile");

            registry.emplace<PixeledTileComponent>(pixelEntity);

            splittedEntities.push_back(pixelEntity);
        }
    }

    return splittedEntities;
}