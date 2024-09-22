#pragma once
#include "utils/factories/game_objects_factory.h"
#include "utils/systems/audio_system.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <queue>
#include <unordered_map>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/systems/box2d_entt_contact_listener.h>
#include <utils/systems/input_event_manager.h>

class PlayerControlSystem
{
    entt::registry& registry;
    InputEventManager& inputEventManager;
    GameOptions& gameState;
    CoordinatesTransformer coordinatesTransformer;
    Box2dBodyCreator box2dBodyCreator;
    Box2dEnttContactListener& contactListener;
    GameObjectsFactory& gameObjectsFactory;
    AudioSystem& audioSystem;
    std::unordered_map<InputEventManager::EventType, std::queue<InputEventManager::EventInfo>> eventsQueueByType;
public:
    PlayerControlSystem(
        EnttRegistryWrapper& registryWrapper, InputEventManager& inputEventManager, Box2dEnttContactListener& contactListener, GameObjectsFactory& gameObjectsFactory,
        AudioSystem& audioSystem);
public: ///////////////////////////////////////////////// Update. ///////////////////////////////////////////////
    void Update(float deltaTime);
    void ProcessEventsQueue(float deltaTime);
    void RestrictPlayerHorizontalSpeed(entt::entity playerEntity);
private: ///////////////////////////////////////////// Subscriptions. ///////////////////////////////////////////
    void SubscribeToInputEvents();
    void SubscribeToContactListener();
private: ////////////////////////////////// Callbacks for the InputEventManager. ////////////////////////////////
    void HandlePlayerMovement(const InputEventManager::EventInfo& eventInfo, float deltaTime);
    void HandlePlayerAttackOnReleaseButton(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerAttackOnHoldButton(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerBuildingAction(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerWeaponDirection(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerChangeWeapon(const InputEventManager::EventInfo& eventInfo);
private: /////////////////// Callback for contact listener. Uses to set the ground contact flag. /////////////////
    void HandlePlayerBeginPlayerContact(const Box2dEnttContactListener::ContactInfo& contactInfo);
    void HandlePlayerEndPlayerContact(const Box2dEnttContactListener::ContactInfo& contactInfo);
    void SetGroundContactFlagIfEntityIsPlayer(entt::entity entity, bool value);
private: //////////////////////////////////////////////// Shooting. //////////////////////////////////////////////
    entt::entity MakeShotIfPossible(entt::entity playerEntity, float throwingForce);
    void UpdateFireRateAndReloadTime(entt::entity playerEntity, float deltaTime);
};