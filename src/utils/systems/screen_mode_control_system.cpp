#include "screen_mode_control_system.h"
#include <SDL_video.h>
#include <utils/systems/input_event_manager.h>

ScreenModeControlSystem::ScreenModeControlSystem(InputEventManager& inputEventManager, SDLWindowRAII& window) : inputEventManager(inputEventManager), window(window)
{
    SubscribeToInputEvents();
}

void ScreenModeControlSystem::SubscribeToInputEvents()
{
    inputEventManager.Subscribe(
        InputEventManager::EventType::ButtonPress,
        [this](const InputEventManager::EventInfo& eventInfo)
        {
            // Change screen mode on F11.
            auto& originalEvent = eventInfo.originalEvent;
            Uint32 flags = SDL_GetWindowFlags(window);
            if (originalEvent.key.keysym.scancode == SDL_SCANCODE_F11)
            {
                if (flags & SDL_WINDOW_FULLSCREEN)
                {
                    SDL_SetWindowFullscreen(window, 0);
                }
                else
                {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                }
            }
        });
}