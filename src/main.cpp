#include <ecs/systems/animation_update_system.h>
#include <ecs/systems/camera_control_system.h>
#include <ecs/systems/debug_system.h>
#include <ecs/systems/events_control_system.h>
#include <ecs/systems/game_logic_system.h>
#include <ecs/systems/map_loader_system.h>
#include <ecs/systems/phisics_systems.h>
#include <ecs/systems/player_control_systems.h>
#include <ecs/systems/render_hud_systems.h>
#include <ecs/systems/render_world_system.h>
#include <ecs/systems/timers_control_system.h>
#include <ecs/systems/weapon_control_system.h>
#include <iostream>
#include <magic_enum.hpp>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/json_utils.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/objects_factory.h>
#include <utils/file_system.h>
#include <utils/logger.h>
// #include <utils/network/steam_networking_init_RAII.h>
#include <utils/resources/resource_manager.h>
#include <utils/sdl/sdl_RAII.h>
#include <utils/sdl/sdl_imgui_RAII.h>
#include <utils/sdl/sdl_primitives_renderer.h>
#include <utils/systems/audio_system.h>
#include <utils/systems/event_queue_system.h>
#include <utils/systems/game_state_control_system.h>
#include <utils/systems/input_event_manager.h>
#include <utils/systems/screen_mode_control_system.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__

// This stuff is needed to support the main loop in Emscripten.
std::function<void()> globalMainLoopLambda;
void mainLoopForEmscripten()
{
    globalMainLoopLambda();
}

int main([[maybe_unused]] int argc, char* args[])
{
    try
    {
        // Set the current directory to the executable directory.
        std::string execPath = args[0];
        std::string execDir = execPath.substr(0, execPath.find_last_of("\\/"));
        std::filesystem::current_path(execDir);

        // Set the paths to the configuration and log files.
        std::filesystem::path configFilePath = "config.json";
        std::filesystem::path logFilePath = "logs/wofares_game_engine.log";

        // Initialize the logger and the configuration.
        utils::Config::InitInstanceFromFile(configFilePath);

        spdlog::level::level_enum logLevel = utils::GetConfig<spdlog::level::level_enum, "main.logLevel">();
#ifdef MY_DEBUG
        if (logLevel > spdlog::level::debug)
            logLevel = spdlog::level::debug;
#endif // MY_DEBUG
        utils::Logger::Init(logFilePath, logLevel);

        // Log initial information.
        MY_LOG(info, "*********************************************");
        MY_LOG(info, "****** Wofares Game Engine started *****");
        MY_LOG(info, "*********************************************");
        MY_LOG(info, "Current directory set to: {}", execDir);
        MY_LOG(info, "Config file loaded: {}", configFilePath.string());

        // #ifndef DisableSteamNetworkingSockets
        //         // Initialize the SteamNetworkingSockets library.
        //         SteamNetworkingInitRAII::Options steamNetworkingOptions;
        //         steamNetworkingOptions.debugSeverity =
        //             utils::GetConfig<ESteamNetworkingSocketsDebugOutputType, "Networking.debugSeverity">();
        //         MY_LOG(info, "[SteamNetworking] debugSeverity: {}", steamNetworkingOptions.debugSeverity);
        //         SteamNetworkingInitRAII steamNetworkingInitRAII(steamNetworkingOptions);
        //         SteamNetworkingInitRAII::SetDebugCallback(
        //             []([[maybe_unused]] ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
        //             { MY_LOG(info, "[SteamNetworking] {}", pszMsg); });
        // #endif // DisableSteamNetworkingSockets

        // Create an EnTT registry.
        entt::registry registry;
        EnttRegistryWrapper registryWrapper(registry);

        // Create a game state entity.
        auto& gameOptions = registry.emplace<GameOptions>(
            registryWrapper.Create("GameOptions"), utils::GetConfig<GameOptions, "GameOptions">());

        // Create a contact listener and subscribe it to the physics world.
        Box2dEnttContactListener contactListener(registryWrapper);

        // Initialize SDL, create a window and a renderer. Initialize ImGui.
        SDLInitializerRAII sdlInitializer(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
        SDLAudioInitializerRAII sdlAudioInitializer;
        SDLWindowRAII window("Wofares Game Engine created by marleeeeeey", gameOptions.windowOptions.windowSize);
        SDLRendererRAII renderer(window.get(), SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        ImGuiSDLRAII imguiSDL(window.get(), renderer.get());

        std::filesystem::path assetsSettingsFilePath = "assets/assets_settings.json";
        auto assetsSettingsJson = utils::LoadJsonFromFile(assetsSettingsFilePath);
        MY_LOG(info, "Assets settings loaded: {}", assetsSettingsFilePath.string());
        ResourceManager resourceManager(renderer.get(), assetsSettingsJson);
        AudioSystem audioSystem(resourceManager);
        audioSystem.PlayMusic("background_music");

        ObjectsFactory objectsFactory(registryWrapper, resourceManager);

        // Create a weapon control system and subscribe it to the contact listener.
        WeaponControlSystem weaponControlSystem(registryWrapper, contactListener, audioSystem, objectsFactory);

        // Create an input event manager and an event queue system.
        InputEventManager inputEventManager;
        EventQueueSystem eventQueueSystem(inputEventManager);

        // Subscribe all systems that need to handle input events.
        PlayerControlSystem playerControlSystem(
            registryWrapper, inputEventManager, contactListener, objectsFactory, audioSystem);
        CameraControlSystem cameraControlSystem(registryWrapper.GetRegistry(), inputEventManager);
        GameStateControlSystem gameStateControlSystem(registryWrapper.GetRegistry(), inputEventManager);

        // Create a systems with no input events.
        SdlPrimitivesRenderer primitivesRenderer(registryWrapper.GetRegistry(), renderer.get());
        PhysicsSystem physicsSystem(registryWrapper);
        RenderWorldSystem RenderWorldSystem(
            registryWrapper.GetRegistry(), renderer.get(), resourceManager, primitivesRenderer);
        RenderHUDSystem RenderHUDSystem(registryWrapper.GetRegistry(), renderer.get(), assetsSettingsJson);

        // Auxiliary systems.
        ScreenModeControlSystem screenModeControlSystem(inputEventManager, window);
        TimersControlSystem timersControlSystem(registryWrapper.GetRegistry());

        // Load the map.
        MapLoaderSystem mapLoaderSystem(registryWrapper, resourceManager, contactListener);

        AnimationUpdateSystem animationUpdateSystem(registryWrapper.GetRegistry(), resourceManager);
        GameLogicSystem gameLogicSystem(registryWrapper.GetRegistry(), objectsFactory, audioSystem);

        EventsControlSystem eventsControlSystem(registryWrapper.GetRegistry());

        DebugSystem debugSystem(registryWrapper.GetRegistry(), objectsFactory);

        // Set the main loop lambda.
        Uint32 lastTick = SDL_GetTicks();
        globalMainLoopLambda = [&]()
        {
            // Calculate delta time.
            Uint32 frameStart = SDL_GetTicks();
            float deltaTime = static_cast<float>(frameStart - lastTick) / 1000.0f;
            lastTick = frameStart;

            if (gameOptions.controlOptions.reloadMap)
            {
                auto level = resourceManager.GetTiledLevel(gameOptions.levelOptions.mapName);
                mapLoaderSystem.LoadMap(level);
                gameOptions.controlOptions.reloadMap = false;
                inputEventManager.Reset();
            }

            // Handle input events.
            eventQueueSystem.Update(deltaTime);

            // Auxiliary systems.
            timersControlSystem.Update(deltaTime);
            eventsControlSystem.Update();

            // Update the physics and post-physics systems to prepare the render.
            physicsSystem.Update(deltaTime);
            playerControlSystem.Update(deltaTime);
            gameLogicSystem.Update(deltaTime);
            weaponControlSystem.Update();
            cameraControlSystem.Update(deltaTime);

            // Update animation.
            animationUpdateSystem.Update(deltaTime);

            debugSystem.Update();

            // Render the scene and the HUD.
            imguiSDL.startFrame();
            RenderWorldSystem.Render();
            RenderHUDSystem.Render();
            imguiSDL.finishFrame();

#ifndef __EMSCRIPTEN__
            // Cap the frame rate.
            Uint32 frameTimeMs = SDL_GetTicks() - frameStart;
            const Uint32 frameDelayMs = 1000 / utils::GetConfig<unsigned, "main.fps">();
            if (frameDelayMs > frameTimeMs)
            {
                SDL_Delay(frameDelayMs - frameTimeMs);
            }
#endif // __EMSCRIPTEN__
        };

        // Run the main loop.
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(mainLoopForEmscripten, 0, 1);
        const Uint32 frameDelayMs = 1000 / utils::GetConfig<unsigned, "main.webFps">();
        emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, frameDelayMs);
#else
        while (!gameOptions.controlOptions.quit)
        {
            globalMainLoopLambda();
        }
#endif // __EMSCRIPTEN__

        registryWrapper.LogAllEntitiesByTheirNames();
    }
    catch (const std::runtime_error& e)
    {
        // This line is needed to log exceptions from the logger. Because the logger is used in the catch block.
        std::cout << "Unhandled exception catched in main: " << e.what() << std::endl;

        MY_LOG(warn, "Unhandled exception catched in main: {}", e.what());
        return -1;
    }

    return 0;
}
