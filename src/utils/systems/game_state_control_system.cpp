#include "game_state_control_system.h"
#include <utils/game_options.h>

GameStateControlSystem::GameStateControlSystem(entt::registry& registry, InputEventManager& inputEventManager)
  : registry(registry), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), inputEventManager(inputEventManager)
{
    SubscribeToInputEvents();
}

void GameStateControlSystem::HandleGameStateChange(const SDL_Event& event)
{
    bool isEscPressed = event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE;

    if (isEscPressed || event.type == SDL_QUIT)
    {
        gameState.controlOptions.quit = true;
    }
}

void GameStateControlSystem::HandleSpaceHoldButtonToDebugInfo(const InputEventManager::EventInfo& eventInfo)
{
    auto& originalEvent = eventInfo.originalEvent;

    if (originalEvent.key.keysym.scancode == SDL_SCANCODE_SPACE)
    {
        gameState.debugInfo.spacePressedDuration = eventInfo.holdDuration;
    }
}

void GameStateControlSystem::HandleSpaceReleaseAfterHoldButtonToDebugInfo(const InputEventManager::EventInfo& eventInfo)
{
    auto& originalEvent = eventInfo.originalEvent;

    if (originalEvent.key.keysym.scancode == SDL_SCANCODE_SPACE)
    {
        gameState.debugInfo.spacePressedDurationOnUpEvent = eventInfo.holdDuration;
    }
}

void GameStateControlSystem::SubscribeToInputEvents()
{
    inputEventManager.Subscribe([this](const InputEventManager::EventInfo& eventInfo) { HandleGameStateChange(eventInfo.originalEvent); });

    inputEventManager.Subscribe(InputEventManager::EventType::ButtonHold, [this](const InputEventManager::EventInfo& eventInfo) { HandleSpaceHoldButtonToDebugInfo(eventInfo); });

    inputEventManager.Subscribe(
        InputEventManager::EventType::ButtonReleaseAfterHold, [this](const InputEventManager::EventInfo& eventInfo) { HandleSpaceReleaseAfterHoldButtonToDebugInfo(eventInfo); });
}