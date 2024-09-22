#include "render_hud_systems.h"
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/rendering_components.h>
#include <imgui.h>
#include <my_cpp_utils/config.h>
#include <utils/game_options.h>
#include <utils/imgui/imgui_RAII.h>
#include <utils/logger.h>
#include <utils/sdl/sdl_colors.h>
#include <utils/sdl/sdl_gfx_wrapper.h>

RenderHUDSystem::RenderHUDSystem(entt::registry& registry, SDL_Renderer* renderer, nlohmann::json assetsSettingsJson)
  : registry(registry), renderer(renderer), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), assetsSettingsJson(assetsSettingsJson)
{}

void RenderHUDSystem::Render()
{
    if (utils::GetConfig<bool, "RenderHUDSystem.showGrid">())
        RenderGrid();

    if (utils::GetConfig<bool, "RenderHUDSystem.debugMenuShow">())
    {
        RenderDebugMenu();
        DrawPlayersWindowInfo();
    }

    if (gameState.showGameInstructions)
    {
        ShowGameInstructions();
    }
    else
    {
        if (gameState.controlOptions.showLevelCompleteScreen)
            ShowLevelCompleteScreen(true);

        if (gameState.controlOptions.showGameOverScreen)
            ShowLevelCompleteScreen(false);
    }
}

void RenderHUDSystem::RenderDebugMenu()
{
    ImGui::Begin("Debug Menu");

    if (ImGui::Button("Reload Map"))
        gameState.controlOptions.reloadMap = true;

    // Caclulare count of tiles, players and dynamic bodies:
    auto tiles = registry.view<TileComponent>();
    auto players = registry.view<PlayerComponent>();
    auto dynamicBodies = registry.view<PhysicsComponent>();
    size_t dynamicBodiesCount = 0;
    for (auto entity : dynamicBodies)
    {
        auto body = dynamicBodies.get<PhysicsComponent>(entity).bodyRAII->GetBody();
        if (body->GetType() == b2_dynamicBody)
            dynamicBodiesCount++;
    }

    // Print the gravity and camera scale, count of tiles, players and dynamic bodies in one line.
    auto gravity = gameState.physicsWorld->GetGravity().Length();
    auto cameraScale = gameState.windowOptions.cameraScale;
    ImGui::TextUnformatted(MY_FMT("{:.2f}/{:.2f} (Gr/Sc)", gravity, cameraScale).c_str());
    ImGui::TextUnformatted(MY_FMT("{}/{}/{} (Ts/Ps/DB)", tiles.size(), players.size(), dynamicBodiesCount).c_str());
    ImGui::TextUnformatted(MY_FMT("Camera center: {}", gameState.windowOptions.cameraCenterSdl).c_str());

    // Print debug info.
    ImGui::TextUnformatted(MY_FMT("Space pressed duration: {:.2f}", gameState.debugInfo.spacePressedDuration).c_str());
    ImGui::TextUnformatted(MY_FMT("Space pressed duration on up event: {:.2f}", gameState.debugInfo.spacePressedDurationOnUpEvent).c_str());

    // Print last mouse position.
    const auto& lastMousePosition = gameState.windowOptions.lastMousePosInWindow;
    ImGui::TextUnformatted(MY_FMT("Last mouse position: {}", lastMousePosition).c_str());

    ImGui::End();
}

void RenderHUDSystem::RenderGrid()
{
    auto& gameState = registry.get<GameOptions>(registry.view<GameOptions>().front());

    const int gridSize = 32;
    const SDL_Color gridColor = GetSDLColor(ColorName::Grey);
    const SDL_Color screenCenterColor = GetSDLColor(ColorName::Red);

    // Get the window size to determine the drawing area
    int windowWidth = static_cast<int>(gameState.windowOptions.windowSize.x);
    int windowHeight = static_cast<int>(gameState.windowOptions.windowSize.y);

    const auto& cameraCenterSdl = gameState.windowOptions.cameraCenterSdl;

    // Calculate the start and end points for drawing the grid
    int startX = static_cast<int>(cameraCenterSdl.x - windowWidth / 2 / gameState.windowOptions.cameraScale);
    int startY = static_cast<int>(cameraCenterSdl.y - windowHeight / 2 / gameState.windowOptions.cameraScale);
    int endX = startX + windowWidth / gameState.windowOptions.cameraScale;
    int endY = startY + windowHeight / gameState.windowOptions.cameraScale;

    // Align the beginning of the grid with the cell boundaries and add 1.4 of the grid size to center the grid.
    // This shift is necessary to align Box2D bodies with the grid.
    startX -= startX % gridSize + gridSize / 4;
    startY -= startY % gridSize + gridSize / 4;

    // Draw vertical grid lines
    SetRenderDrawColor(renderer, gridColor);
    for (int x = startX; x <= endX; x += gridSize)
    {
        int screenX = static_cast<int>((x - cameraCenterSdl.x) * gameState.windowOptions.cameraScale + windowWidth / 2);
        SDL_RenderDrawLine(renderer, screenX, 0, screenX, windowHeight);
    }

    // Draw horizontal grid lines
    for (int y = startY; y <= endY; y += gridSize)
    {
        int screenY = static_cast<int>((y - cameraCenterSdl.y) * gameState.windowOptions.cameraScale + windowHeight / 2);
        SDL_RenderDrawLine(renderer, 0, screenY, windowWidth, screenY);
    }

    // Draw the center of screen point
    DrawCross(renderer, gameState.windowOptions.windowSize / 2.0f, 20, screenCenterColor);
}

void RenderHUDSystem::DrawPlayersWindowInfo()
{
    for (auto entity : registry.view<PlayerComponent>())
    {
        auto& playerInfo = registry.get<PlayerComponent>(entity);
        ImGui::Begin(MY_FMT("Player {} HUD", playerInfo.number).c_str());

        // Encrease font size.
        ImGui::SetWindowFontScale(3);

        std::string reloadingStatus = "";
        if (playerInfo.weapons.at(playerInfo.currentWeapon).remainingReloadTime > 0)
            reloadingStatus = "Reloading...";
        else if (playerInfo.weapons.at(playerInfo.currentWeapon).remainingFireRate > 0)
            reloadingStatus = "Fire rate...";

        ImGui::TextUnformatted(MY_FMT(
                                   "{}, Ammo: {}/{} {}", playerInfo.currentWeapon, playerInfo.weapons.at(playerInfo.currentWeapon).ammoInClip,
                                   playerInfo.weapons.at(playerInfo.currentWeapon).ammoInStorage, reloadingStatus)
                                   .c_str());

        ImGui::End();
    }
}
void RenderHUDSystem::ShowGameInstructions()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(gameState.windowOptions.windowSize.x, gameState.windowOptions.windowSize.y));

    bool open = true;
    ImGui::Begin(
        "Game Instructions", &open,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

    ImGui::SetWindowFontScale(2.5);

    ImGui::TextWrapped(
        "Welcome to the `Wofares Game Engine` created by marleeeeeey. "
        "Visit the game repository on GitHub:\nhttps://github.com/marleeeeeey/wofares_game_engine");
    ImGui::Separator();

    ImGui::TextWrapped(
        "Hungry Portals (Wofares Game Engine based) to the dark realm are desperate for fresh blood. They've reached your home and swallowed your family. "
        "You stand alone. Yet, there is a chance to save them. These portals consume everything they can lift, so destroy everything around you. "
        "Let them choke and burst, and then your family will return. Good luck!");
    ImGui::Separator();

    ImGui::Text("Controls");
    ImGui::BulletText("Move: WASD");
    ImGui::BulletText("Jump: Spacebar");
    ImGui::BulletText("Shoot: Mouse Left Button");
    ImGui::BulletText("Build: Mouse Right Button");
    ImGui::BulletText("Switch to Bazooka: Key 1");
    ImGui::BulletText("Switch to Grenade: Key 2");
    ImGui::BulletText("Scale: Mouse Wheel");
    ImGui::BulletText("Toggle Fullscreen: F11");
    ImGui::BulletText("Drag the screen: Hold Mouse Middle Button");
    ImGui::BulletText("Quit game: ESC");

    ImGui::Separator();
    ImGui::TextWrapped("Enjoy the game and make sure to complete all the missions!");

    ImGui::Separator();
    const char* buttonText = "Start Game";
    ImVec2 textSize = ImGui::CalcTextSize(buttonText);
    float buttonWidth = textSize.x + ImGui::GetStyle().FramePadding.x * 2; // Add some padding
    float windowWidth = ImGui::GetWindowSize().x;
    float positionX = (windowWidth - buttonWidth) * 0.5f;
    ImGui::SetCursorPosX(positionX);
    if (ImGui::Button(buttonText, ImVec2(buttonWidth, textSize.y)))
    {
        gameState.showGameInstructions = false;
        gameState.controlOptions.showLevelCompleteScreen = false;
        gameState.controlOptions.showGameOverScreen = false;
        gameState.controlOptions.reloadMap = true;
    }

    ImGui::End();
}

void RenderHUDSystem::ShowLevelCompleteScreen(bool isWin)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(gameState.windowOptions.windowSize.x, gameState.windowOptions.windowSize.y));

    bool open = true;
    ImGui::Begin(
        "Level Complete", &open,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

    ImGui::SetWindowFontScale(3);

    if (isWin)
    {
        ImGui::TextWrapped("Congratulations! You have completed the level!");
    }
    else
    {
        ImGui::TextWrapped("Game Over! You have lost!");
    }

    ImGui::Separator();

    // Draw buttons "Restart" and "Next Level".
    const char* restartText = "Restart";
    ImVec2 textSize = ImGui::CalcTextSize(restartText);
    float buttonWidth = textSize.x + ImGui::GetStyle().FramePadding.x * 2; // Add some padding
    float windowWidth = ImGui::GetWindowSize().x;
    float positionX = (windowWidth - buttonWidth) * 0.5f;
    ImGui::SetCursorPosX(positionX);
    if (ImGui::Button(restartText, ImVec2(buttonWidth, textSize.y)))
    {
        gameState.controlOptions.reloadMap = true;
        gameState.controlOptions.showLevelCompleteScreen = false;
        gameState.controlOptions.showGameOverScreen = false;
    }

    const char* nextLevelText = "Next Level";
    textSize = ImGui::CalcTextSize(nextLevelText);
    buttonWidth = textSize.x + ImGui::GetStyle().FramePadding.x * 2; // Add some padding
    positionX = (windowWidth - buttonWidth) * 0.5f;
    ImGui::SetCursorPosX(positionX);
    if (ImGui::Button(nextLevelText, ImVec2(buttonWidth, textSize.y)))
    {
        auto maps = assetsSettingsJson["maps"];

        // Find the current map in the list of maps.
        auto currentMapName = gameState.levelOptions.mapName;
        auto currentMapIt = std::find_if(maps.begin(), maps.end(), [&currentMapName](const auto& map) { return map["name"] == currentMapName; });

        // Find the next map in the list of maps.
        auto nextMapIt = currentMapIt + 1;
        if (nextMapIt == maps.end())
            nextMapIt = maps.begin();

        // Load the next map.
        gameState.levelOptions.mapName = nextMapIt->at("name").get<std::string>();

        gameState.controlOptions.reloadMap = true;
        gameState.controlOptions.showLevelCompleteScreen = false;
        gameState.controlOptions.showGameOverScreen = false;
    }

    ImGui::End();
}
