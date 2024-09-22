#include "player_control_systems.h"
#include "my_cpp_utils/logger.h"
#include "utils/math_utils.h"
#include <SDL.h>
#include <ecs/components/animation_components.h>
#include <ecs/components/event_components.h>
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <entt/entt.hpp>
#include <glm/fwd.hpp>
#include <imgui_impl_sdl2.h>
#include <my_cpp_utils/config.h>
#include <unordered_map>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/logger.h>
#include <utils/systems/input_event_manager.h>

PlayerControlSystem::PlayerControlSystem(
    EnttRegistryWrapper& registryWrapper, InputEventManager& inputEventManager, Box2dEnttContactListener& contactListener, GameObjectsFactory& gameObjectsFactory,
    AudioSystem& audioSystem)
  : registry(registryWrapper), inputEventManager(inputEventManager), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    coordinatesTransformer(registry), box2dBodyCreator(registry), contactListener(contactListener), gameObjectsFactory(gameObjectsFactory), audioSystem(audioSystem)
{
    SubscribeToInputEvents();
    SubscribeToContactListener();
}

void PlayerControlSystem::Update(float deltaTime)
{
    const auto& players = registry.view<PlayerComponent>();
    for (auto entity : players)
    {
        ProcessEventsQueue(deltaTime);
        UpdateFireRateAndReloadTime(entity, deltaTime);
        RestrictPlayerHorizontalSpeed(entity);
    }
}

void PlayerControlSystem::ProcessEventsQueue(float deltaTime)
{
    for (auto& [eventType, eventsQueue] : eventsQueueByType)
    {
        while (!eventsQueue.empty())
        {
            auto eventInfo = eventsQueue.front();
            eventsQueue.pop();
            if (eventType == InputEventManager::EventType::ButtonHold)
            {
                HandlePlayerMovement(eventInfo, deltaTime);
                HandlePlayerChangeWeapon(eventInfo);
                HandlePlayerAttackOnHoldButton(eventInfo);
            }
            else if (eventType == InputEventManager::EventType::ButtonReleaseAfterHold)
            {
                HandlePlayerAttackOnReleaseButton(eventInfo);
            }
            else if (eventType == InputEventManager::EventType::RawSdlEvent)
            {
                HandlePlayerBuildingAction(eventInfo);
                HandlePlayerWeaponDirection(eventInfo);
            }
        }
    }
}

void PlayerControlSystem::RestrictPlayerHorizontalSpeed(entt::entity playerEntity)
{
    const auto& [player, physicalBody] = registry.get<PlayerComponent, PhysicsComponent>(playerEntity);
    auto body = physicalBody.bodyRAII->GetBody();

    auto velocity = body->GetLinearVelocity();
    float maxHorizontalSpeed = utils::GetConfig<float, "PlayerControlSystem.maxHorizontalSpeed">();

    if (std::abs(velocity.x) > maxHorizontalSpeed)
    {
        velocity.x = std::copysign(maxHorizontalSpeed, velocity.x);
        body->SetLinearVelocity(velocity);
    }
}

void PlayerControlSystem::SubscribeToInputEvents()
{
    // TODO2: Think. Maybe move this logic to the InputEventManager.
    inputEventManager.Subscribe(
        InputEventManager::EventType::ButtonHold,
        [this](const InputEventManager::EventInfo& eventInfo) { eventsQueueByType[InputEventManager::EventType::ButtonHold].push(eventInfo); });

    inputEventManager.Subscribe(
        InputEventManager::EventType::ButtonReleaseAfterHold,
        [this](const InputEventManager::EventInfo& eventInfo) { eventsQueueByType[InputEventManager::EventType::ButtonReleaseAfterHold].push(eventInfo); });

    inputEventManager.Subscribe(
        InputEventManager::EventType::RawSdlEvent,
        [this](const InputEventManager::EventInfo& eventInfo) { eventsQueueByType[InputEventManager::EventType::RawSdlEvent].push(eventInfo); });
}

void PlayerControlSystem::SubscribeToContactListener()
{
    contactListener.SubscribeContact(
        Box2dEnttContactListener::ContactType::BeginSensor, [this](const Box2dEnttContactListener::ContactInfo& contactInfo) { HandlePlayerBeginPlayerContact(contactInfo); });
    contactListener.SubscribeContact(
        Box2dEnttContactListener::ContactType::EndSensor, [this](const Box2dEnttContactListener::ContactInfo& contactInfo) { HandlePlayerEndPlayerContact(contactInfo); });
}

void PlayerControlSystem::HandlePlayerMovement(const InputEventManager::EventInfo& eventInfo, float deltaTime)
{
    auto& originalEvent = eventInfo.originalEvent;

    const auto& players = registry.view<PlayerComponent, PhysicsComponent>();
    for (auto entity : players)
    {
        const auto& [player, physicalBody] = players.get<PlayerComponent, PhysicsComponent>(entity);
        auto body = physicalBody.bodyRAII->GetBody();

        bool allowLeftRightMovement = utils::GetConfig<bool, "PlayerControlSystem.allowLeftRightMovementInAir">() || player.OnGround();

        auto mass = body->GetMass();

        if (originalEvent.key.keysym.scancode == SDL_SCANCODE_W || originalEvent.key.keysym.scancode == SDL_SCANCODE_SPACE)
        {
            if (player.OnGround() || player.allowJumpForceInAir)
            {
                float jumpForce = 1900.0f * mass * deltaTime;

                auto timeEventComponent = registry.try_get<TimeEventComponent>(entity);

                if (player.OnGround())
                {
                    player.allowJumpForceInAir = false;
                    // remove time event component if it is present
                    if (timeEventComponent)
                        registry.remove<TimeEventComponent>(entity);
                }
                else
                {
                    // If time event component is present then it impact on the player jump force.
                    // Less time to activation - less jump force.
                    if (timeEventComponent && timeEventComponent->isActivated)
                        jumpForce *= std::pow(timeEventComponent->timeToActivation, 1);
                }

                body->ApplyForceToCenter(b2Vec2(0, -jumpForce), true);

                if (!player.allowJumpForceInAir)
                {
                    player.allowJumpForceInAir = true;
                    registry.emplace_or_replace<TimeEventComponent>(
                        entity, 0.13f,
                        [this](entt::entity playerEntity)
                        {
                            auto& playerComponent = registry.get<PlayerComponent>(playerEntity);
                            playerComponent.allowJumpForceInAir = false;
                        });
                }
            }
        }

        if (allowLeftRightMovement)
        {
            float movingForce = 600.0f * mass * deltaTime;

            if (originalEvent.key.keysym.scancode == SDL_SCANCODE_A)
            {
                MY_LOG(trace, "Player {} moved left", player.number);
                body->ApplyForceToCenter(b2Vec2(-movingForce, 0), true);
            }

            if (originalEvent.key.keysym.scancode == SDL_SCANCODE_D)
            {
                MY_LOG(trace, "Player {} moved right", player.number);
                body->ApplyForceToCenter(b2Vec2(movingForce, 0), true);
            }
        }
    }
}

void PlayerControlSystem::HandlePlayerAttackOnReleaseButton(const InputEventManager::EventInfo& eventInfo)
{
    if (eventInfo.originalEvent.button.button == SDL_BUTTON_LEFT)
    {
        const auto& players = registry.view<PlayerComponent, PhysicsComponent, AnimationComponent>();
        for (auto entity : players)
        {
            // TODO2: Make in no linear way.
            float throwingForce = std::min(eventInfo.holdDuration * 0.3f, 0.2f);
            MY_LOG(debug, "Throwing force: {}", throwingForce);
            MakeShotIfPossible(entity, throwingForce);
        }
    }
}

void PlayerControlSystem::HandlePlayerAttackOnHoldButton(const InputEventManager::EventInfo& eventInfo)
{
    if (eventInfo.originalEvent.button.button == SDL_BUTTON_LEFT)
    {
        const auto& players = registry.view<PlayerComponent, PhysicsComponent, AnimationComponent>();
        for (auto entity : players)
        {
            float throwingForce = 0.0f;
            MakeShotIfPossible(entity, throwingForce);
        }
    }
}

void PlayerControlSystem::HandlePlayerBuildingAction(const InputEventManager::EventInfo& eventInfo)
{
    auto event = eventInfo.originalEvent;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
    {
        glm::vec2 posWindow = glm::vec2(event.button.x, event.button.y);
        glm::vec2 posWorld = coordinatesTransformer.ScreenToWorld(posWindow);
        gameObjectsFactory.SpawnBuildingBlock(posWorld);
    }
}

void PlayerControlSystem::HandlePlayerWeaponDirection(const InputEventManager::EventInfo& eventInfo)
{
    auto event = eventInfo.originalEvent;

    if (event.type == SDL_MOUSEMOTION)
    {
        const auto& players = registry.view<PlayerComponent, PhysicsComponent>();
        for (auto entity : players)
        {
            const auto& [playerInfo, physicalBody] = players.get<PlayerComponent, PhysicsComponent>(entity);
            auto playerBody = physicalBody.bodyRAII->GetBody();

            glm::vec2 mousePosScreen{event.motion.x, event.motion.y};
            glm::vec2 playerPosScreen = coordinatesTransformer.PhysicsToScreen(playerBody->GetPosition());
            glm::vec2 directionVec = mousePosScreen - playerPosScreen;
            playerInfo.weaponDirection = glm::normalize(directionVec);
        }
    }
}

void PlayerControlSystem::HandlePlayerChangeWeapon(const InputEventManager::EventInfo& eventInfo)
{
    auto event = eventInfo.originalEvent;

    if (event.type != SDL_KEYDOWN)
        return;

    auto weaponIndex = event.key.keysym.sym - SDLK_1; // Get zero-based index of the weapon.
    auto weaponEnumRange = magic_enum::enum_values<WeaponType>();

    if (weaponIndex < 0 || weaponIndex >= static_cast<int>(weaponEnumRange.size()))
        return;

    const auto& players = registry.view<PlayerComponent, PhysicsComponent>();
    for (auto entity : players)
    {
        auto& playerInfo = players.get<PlayerComponent>(entity);
        auto newWeapon = weaponEnumRange[weaponIndex];

        if (newWeapon == playerInfo.currentWeapon)
            continue;

        if (!playerInfo.weapons.contains(newWeapon))
        {
            MY_LOG(warn, "Player {} does not have {} weapon", playerInfo.number, newWeapon);
            continue;
        }

        playerInfo.currentWeapon = weaponEnumRange[weaponIndex];
        MY_LOG(trace, "Player {} changed weapon to {}", playerInfo.number, playerInfo.currentWeapon);
    }
}

void PlayerControlSystem::HandlePlayerEndPlayerContact(const Box2dEnttContactListener::ContactInfo& contactInfo)
{
    SetGroundContactFlagIfEntityIsPlayer(contactInfo.entityA, false);
    SetGroundContactFlagIfEntityIsPlayer(contactInfo.entityB, false);
}

void PlayerControlSystem::HandlePlayerBeginPlayerContact(const Box2dEnttContactListener::ContactInfo& contactInfo)
{
    SetGroundContactFlagIfEntityIsPlayer(contactInfo.entityA, true);
    SetGroundContactFlagIfEntityIsPlayer(contactInfo.entityB, true);
}

void PlayerControlSystem::SetGroundContactFlagIfEntityIsPlayer(entt::entity entity, bool value)
{
    auto playerInfo = registry.try_get<PlayerComponent>(entity);
    if (playerInfo)
    {
        playerInfo->countOfGroundContacts += value ? 1 : -1;
        MY_LOG(trace, "Player {} countOfGroundContacts: {}", playerInfo->number, playerInfo->countOfGroundContacts);
    }
}

entt::entity PlayerControlSystem::MakeShotIfPossible(entt::entity playerEntity, float throwingForce)
{
    if (!registry.all_of<PlayerComponent, PhysicsComponent, AnimationComponent>(playerEntity))
    {
        MY_LOG(trace, "[MakeShotIfPossible] entity does not have all of the required components. Entity: {}", playerEntity);
        return entt::null;
    }

    auto& playerInfo = registry.get<PlayerComponent>(playerEntity);

    // Check if the throwing force is zero for the grenade.
    if (throwingForce <= 0 && (playerInfo.currentWeapon == WeaponType::Grenade || playerInfo.currentWeapon == WeaponType::Bazooka))
    {
        MY_LOG(trace, "[MakeShotIfPossible] Throwing force shouldn't be zero for weapon {}. Entity: {}, force: {}", playerInfo.currentWeapon, playerEntity, throwingForce);
        return entt::null;
    }

    // Check if player has weapon set as current.
    if (!playerInfo.weapons.contains(playerInfo.currentWeapon))
    {
        MY_LOG(trace, "[MakeShotIfPossible] Player does not have {} weapon set as current. Entity: {}", playerInfo.currentWeapon, static_cast<int>(playerEntity));
        return entt::null;
    }
    WeaponProps& currentWeaponProps = playerInfo.weapons.at(playerInfo.currentWeapon);

    // Check if player has ammo for the weapon.
    if (currentWeaponProps.ammoInClip == 0)
    {
        MY_LOG(trace, "[MakeShotIfPossible] Player does not have ammo in clip for the {} weapon. Entity: {}", playerInfo.currentWeapon, static_cast<int>(playerEntity));
        return entt::null;
    }

    // Check if player is in the reload process.
    if (currentWeaponProps.remainingReloadTime > 0)
    {
        MY_LOG(warn, "[MakeShotIfPossible] Player is in the reload process. Entity: {}", static_cast<int>(playerEntity));
        return entt::null;
    }

    // Check if player in the fire rate cooldown.
    if (currentWeaponProps.remainingFireRate > 0)
    {
        MY_LOG(trace, "[MakeShotIfPossible] Player is in the fire rate cooldown. Entity: {}", static_cast<int>(playerEntity));
        return entt::null;
    }

    // Update player ammo.
    currentWeaponProps.ammoInClip -= 1;
    currentWeaponProps.remainingFireRate = currentWeaponProps.fireRate;
    if (currentWeaponProps.ammoInClip == 0)
    {
        // TODO2: Start reloading sound and animation.
        currentWeaponProps.remainingReloadTime = currentWeaponProps.reloadTime;
    }

    // Calculate initial bullet speed.
    float initialBulletSpeed = currentWeaponProps.bulletEjectionForce / currentWeaponProps.bulletMass;
    initialBulletSpeed += throwingForce; // Add throwing force for the grenade with zero initial speed.
    initialBulletSpeed *= 40; // TODO2: Remove this magic number.

    // Check if player has weapon set as current.
    if (!playerInfo.weapons.contains(playerInfo.currentWeapon))
    {
        MY_LOG(trace, "[CreateBullet] Player does not have {} weapon set as current. Entity: {}", playerInfo.currentWeapon, playerEntity);
        return entt::null;
    }
    const WeaponProps& weaponProps = playerInfo.weapons.at(playerInfo.currentWeapon);

    // Caclulate initial bullet position.
    const auto& playerBody = registry.get<PhysicsComponent>(playerEntity).bodyRAII->GetBody();
    const auto& playerAnimationComponent = registry.get<AnimationComponent>(playerEntity);
    glm::vec2 playerSizeWorld = playerAnimationComponent.GetHitboxSize();
    glm::vec2 playerPosWorld = coordinatesTransformer.PhysicsToWorld(playerBody->GetPosition());
    auto weaponInitialPointShift = playerInfo.weaponDirection * (playerSizeWorld.x) / 2.0f;
    glm::vec2 initialPosWorld = playerPosWorld + weaponInitialPointShift;

    // Create a bullet.
    float angle = utils::GetAngleFromDirection(playerInfo.weaponDirection);
    auto bulletEntity = gameObjectsFactory.SpawnBullet(initialPosWorld, initialBulletSpeed, angle, weaponProps);

    audioSystem.PlaySoundEffect(currentWeaponProps.shotSoundName);

    return bulletEntity;
}

void PlayerControlSystem::UpdateFireRateAndReloadTime(entt::entity playerEntity, float deltaTime)
{
    auto& playerInfo = registry.get<PlayerComponent>(playerEntity);
    for (auto& [weaponType, weaponProps] : playerInfo.weapons)
    {
        if (weaponProps.remainingFireRate > 0)
        {
            weaponProps.remainingFireRate -= deltaTime;
        }

        if (weaponProps.remainingReloadTime > 0)
        {
            weaponProps.remainingReloadTime -= deltaTime;

            if (weaponProps.remainingReloadTime <= 0)
            {
                // TODO2: Stop reloading sound and animation.
                weaponProps.ammoInClip = std::min(weaponProps.clipSize, weaponProps.ammoInStorage);
                weaponProps.ammoInStorage -= weaponProps.ammoInClip;
                MY_LOG(
                    trace, "Player {} reloaded weapon {}. Ammo in clip: {}, ammo in storage: {}", playerInfo.number, weaponType, weaponProps.ammoInClip, weaponProps.ammoInStorage);
            }
        }
    }
}
