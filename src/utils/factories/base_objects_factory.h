#pragma once
#include <ecs/components/animation_components.h>
#include <ecs/components/rendering_components.h>
#include <entt/entt.hpp>
#include <utils/box2d/box2d_body_tuner.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/factories/components_factory.h>
#include <utils/game_options.h>
#include <utils/resources/resource_manager.h>
#include <utils/sdl/sdl_texture_process.h>

class BaseObjectsFactory
{
    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    GameOptions& gameState;
    Box2dBodyCreator box2dBodyCreator;
    CoordinatesTransformer coordinatesTransformer;
    Box2dBodyTuner bodyTuner;
    ComponentsFactory& componentsFactory;
public:
    BaseObjectsFactory(EnttRegistryWrapper& registryWrapper, ComponentsFactory& componentsFactory);

    enum class SpawnPolicyBase
    {
        // Spawn the one object when the function is called first time.
        This,
        // Spawn the new object every frame. Don't delete the old objects.
        All,
        // Spawn the new object every frame. Delete the old objects if limit is reached.
        Last,
        // Spawn the new object every frame. Delete the new objects if limit is reached.
        First,
    };

    struct DebugSpawnOptions
    {
        size_t trailSize = 10;
        SpawnPolicyBase spawnPolicy = SpawnPolicyBase::This;
    };
public: ////////////////////////////////////////////// Main game objects. ////////////////////////////////////////
    entt::entity SpawnTile(glm::vec2 posWorld, float sizeWorld, const TextureRect& textureRect, SpawnTileOption tileOptions, const std::string& name = "Tile");
public: ///////////////////////////////////////// Debug visual objects. //////////////////////////////////////////
    // `nameAsKey` is used as a key in entt registry to search in NameComponent.
    entt::entity SpawnDebugVisualObject(
        const glm::vec2& posWorld, const glm::vec2& sizeWorld, float angle, const std::string& nameAsKey, const DebugSpawnOptions& debugSpawnOptions);
    // `nameAsKey` is used as a key in entt registry to search in NameComponent.
    entt::entity SpawnDebugVisualObject(entt::entity entity, const std::string& nameAsKey, const DebugSpawnOptions& debugSpawnOptions);
public: //////////////////////////////////////////////// Explosions. //////////////////////////////////////////////
    // Split physical entities into smaller ones. Return new entities. Used for explosion effect.
    std::vector<entt::entity> SpawnSplittedPhysicalEnteties(const std::vector<entt::entity>& entities, SDL_Point cellSizeWorld);
    std::vector<entt::entity> SpawnFragmentsAfterExplosion(glm::vec2 centerWorld, float radiusWorld);
public: /////////////////////////////////////////// Explosions. Helpers. /////////////////////////////////////////
    entt::entity SpawnFragmentAfterExplosion(const glm::vec2& posWorld);
public: ///////////////////////////////////////////// Common. Helpers. ///////////////////////////////////////////
    entt::entity SpawnFlyingEntity(const glm::vec2& posWorld, const glm::vec2& sizeWorld, float forceDirection, float force, Box2dBodyOptions::AnglePolicy anglePolicy);
};