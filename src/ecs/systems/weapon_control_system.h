#pragma once
#include "utils/factories/base_objects_factory.h"
#include <entt/entt.hpp>
#include <utils/box2d/box2d_body_tuner.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/game_objects_factory.h>
#include <utils/game_options.h>
#include <utils/systems/audio_system.h>
#include <utils/systems/box2d_entt_contact_listener.h>

class WeaponControlSystem
{
    struct ExplosionEntityWithContactPoint
    {
        entt::entity explosionEntity;
        std::optional<b2Vec2> contactPointPhysics;
    };

    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    GameOptions& gameState;
    Box2dEnttContactListener& contactListener;
    AudioSystem& audioSystem;
    BaseObjectsFactory& baseObjectsFactory;
    CoordinatesTransformer coordinatesTransformer;
    Box2dBodyTuner physicsBodyTuner;
private: /////////////// Queues for entities that should be processed when Box2D calc step has complete. /////////////
    std::map<entt::entity, ExplosionEntityWithContactPoint> explosionEntitiesQueue;
    std::set<entt::entity> becomeStaticEntitiesQueue;
public:
    WeaponControlSystem(EnttRegistryWrapper& registryWrapper, Box2dEnttContactListener& contactListener, AudioSystem& audioSystem, BaseObjectsFactory& baseObjectsFactory);
    void Update(float deltaTime);
private:
    void SubscribeToContactEvents();
    void AppendToExplosionQueue(const ExplosionEntityWithContactPoint& explosionEntityWithContactPoint);
private:
    void CheckTimerExplosionEntities();
    void ProcessEntitiesQueues();
    void DoExplosion(const ExplosionEntityWithContactPoint& explosionEntityWithContactPoint);
    void UpdateFireRateComponents(float deltaTime);
};