# Wofares Game Engine

- [Overview](#overview)
- [Tools](#tools)
- [Guidelines](#guidelines)
  - [Development Guidelines](#development-guidelines)
  - [Assets Editing Guidelines](#assets-editing-guidelines)
  - [Comments](#comments)
- [Build](#build)
  - [Windows build](#windows-build)
  - [Linux build](#linux-build)
  - [Web build](#web-build)
- [Change Log](#change-log)

## Overview

**WOFARES** - **W**orld **OF** squ**ARES** is a platformer game engine innovatively designed around the concept of level destruction. It features a unique mechanism where tiles shatter into pieces, with fragments dispersing realistically thanks to the integration of the Box2D physics engine. This game engine is envisioned to support hot seat gameplay and single-player mode.

At the core of WOFARES's engine lies a robust foundation constructed using C++ and leveraging libraries such as SDL, EnTT, gml, imgui, and Box2D. A pivotal aspect of the game's design is the extensive application of the Entity Component System (ECS) pattern. This architectural approach significantly reduces component coupling and simplifies engine maintenance, ensuring a seamless and immersive gaming experience.

Here is a link to the game created with this engine for [LD55](https://ldjam.com/events/ludum-dare/55/ld55-hungry-portals) championship: [Hungry Portals (Web/Desktop)](https://marleeeeeey.itch.io/ld55-hungry-portals). Source code for the game is available [here](https://github.com/marleeeeeey/LD55_Hungry_Portals).

![game screenshot](docs/wofares_game_engine_screenshot.png)

## Tools

- [Aseprite](https://www.aseprite.org/) - Animated sprite editor & pixel art tool.
- [Tiled](https://www.mapeditor.org/) - Flexible level editor.
- [Box2D](https://box2d.org/) - A 2D physics engine for games.
- [SDL](https://www.libsdl.org/) - Simple DirectMedia Layer.
- [EnTT](https://github.com/skypjack/entt) - A fast and reliable entity-component system (ECS).
- [sfxr](https://sfxr.me/) - Sound effect generator.

## Guidelines

### Development Guidelines

#### Efficient Component Access (Entt library)

- **Use of Views**: When you need to iterate over entities that share a common set of components, prefer using `view` over `all_of` or `any_of` checks within loop iterations. Views are optimized for fast access and iteration, as they precompute and cache the entities that match the specified component criteria. This approach significantly reduces overhead and improves performance, especially with large entity sets.

```cpp
  // Recommended
  auto view = registry.view<ComponentA, ComponentB>();
  for (auto entity : view) {
      // Process entities that have both ComponentA and ComponentB
  }
```

- **Avoid Frequent Component Checks**: Using `registry.all_of` or `registry.any_of` in tight loops for large numbers of entities can be inefficient. These functions check each entity's component makeup at runtime, which can lead to performance degradation if used improperly. (TODO2: check the game code for this).

```cpp
  // Not recommended for large sets or frequent updates
  for (auto entity : registry) {
      if (registry.all_of<ComponentA, ComponentB>(entity)) {
          // Process entity
      }
  }
```

- **Entity Processing Recommendations**: If specific conditional checks on entity components are necessary outside of views, consider structuring your logic to minimize the frequency and scope of these checks, or use architectural patterns that naturally segregate entities into manageable sets. (TODO2: thinking about splitting entities into more specific components).

#### Object Factory

- **ObjectsFactory** is the only object that should add new components to the `entt::registry`.
  - Any method that creates a new object must start with `Spawn`.

#### Component Naming Conventions

- Class names for components must end with `Component`.
  - Header files containing components should end with `_components.h`.

#### Coordinate Systems and Object Sizing

- Texture sizes are measured in pixels, while the sizes of objects in the physical world are measured in meters.
- There are three coordinate systems: screen, pixel world, and physical world. Variable names should end with `Screen`, `World` (TODO3: rename to `Pixels`), or `Physics`.

#### C++ Style Guide

- Prefer using `enum class` over boolean variables.
- Favor composition over inheritance.
- Use descriptive and clear naming conventions that reflect the purpose and usage of the variable or function.

#### Configuration Style Guide

- In `config.json`, section names should correspond to the class names that use them.
- Variable naming in `config.json` should be a direct copy of the variable names in the code.
- Debug options should start with `debug` and may be located in every section.

#### Animation Editor and Engine Integration

- The player animation stores the BBox (bounding box) of the object used for collision detection.
- Bullet animations should not consider the dimensions of the BBox for speed calculations.

### Assets Editing Guidelines

#### Aseprite Guidelines

- **Tagging**: Ensure that all frames are tagged, as these tags are referenced in the C++ code for animation handling.
- **Hitbox Frame**: Include a specifically named frame `Hitbox` within your Aseprite files, which will be used to define interaction boundaries in the game.

#### Tiled Map Editor Guidelines

- **Layer Structure**: Your maps should be organized with the following layers from bottom to top:
  - `background`: For non-interactive scenery that appears behind all game entities.
  - `interiors`: For elements like indoor furnishings that players can potentially interact with but are not part of the terrain.
  - `terrain`: For the main walkable and interactive layer of the game environment.
  - `objects`: For movable or interactive objects that players can interact with during the game.

### Comments

- **Code Block Comments**: Surround the comments for a block of code with repeating // or # symbols depending on the programming language. This practice helps in maintaining visibility of the block's purpose when code folding is enabled in the editor.

```js

# #################### Setup compiler options #######################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generate compile_commands.json.

public: ///////////////////////////////////////// Damage. /////////////////////////////////////////
    size_t damageRadiusWorld = 10; // Radius of the damage in pixels.
    float damageForce = 0.5; // Force of the damage.
public: ///////////////////////////////////////// Ammo. ///////////////////////////////////////////
    size_t ammoInStorage = 100; // Current number of bullets except in the clip.
    size_t ammoInClip = 10; // Current number of bullets in the clip.
    size_t clipSize = 10; // Max number of bullets in the clip.

```

## Build

Wofares Game Engine has been developed, built, and tested on **Windows** and **Web(Emscripten)**. Build also works on **Linux(Ubuntu)** but not tested yet.

### Windows build

#### Clone the Repository

```bash
git clone --recursive https://github.com/marleeeeeey/wofares-game-engine.git
```

#### Prerequisites for Building the Project (Windows)

- CMake.
- Ninja.
- Clang compiler (other compilers may work, but are not officially supported).
- Git (to load submodules).
- Visual Studio Code (recommended for development).
  - Clangd extension (recommended for code analysis).
- Python and 7zip (for packaging game to achive).

#### Build, run and debug via VSCode tasks (Windows)

- Setup user friendly options via editing file [scripts/vscode_tasks_generator.py](scripts/vscode_tasks_generator.py).
- Run the script [scripts/vscode_tasks_generator.py](scripts/vscode_tasks_generator.py) to generate VSCode tasks with your options.
- Open the project folder in VSCode.
- Run task: `003. Install vcpkg as subfolder`.
- Run task: `050. + Run`.
- For debugging press `F5`.

#### Build, run and debug manually (Windows)

To build Wofares Game Engine on Windows, it's recommended to obtain the dependencies by using vcpkg. The following instructions assume that you will follow the vcpkg recommendations and install vcpkg as a subfolder. If you want to use "classic mode" or install vcpkg somewhere else, you're on your own.

This project define it's dependences:
1. In a `vcpkg.json` file, and you are pulling in vcpkg's cmake toolchain file.
2. As git submodules in the `thirdparty` directory. Because some of the libraries not available in vcpkg or have an error in the vcpkg port file.

First, we bootstrap a project-specific installation of vcpkg ("manifest mode") in the default location, `<project root>/vcpkg`. From the project root, run these commands:

```bash
cd wofares_game_engine
# Branch 2024.06.15 is used because of error on the latest master.
# see: https://github.com/microsoft/vcpkg/issues/40912
git clone --branch 2024.06.15 --single-branch https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
```

Now we ask vcpkg to install of the dependencies for our project, which are described by the file `<project root>/vcpkg.json`.  Note that this step is optional, as cmake will automatically do this.  But here we are doing it in a separate step so that we can isolate any problems, because if problems happen here don't have anything to do with your cmake files.

```bash
.\vcpkg\vcpkg install --triplet=x64-windows
```

Next build the project files. There are different options for
1. Telling cmake how to integrate with vcpkg: here we use `CMAKE_TOOLCHAIN_FILE` on the command line.
2. Select Ninja project generator.
3. Select Clang compiler.
4. Enable `CMAKE_EXPORT_COMPILE_COMMANDS` to generate a `compile_commands.json` file for clangd.

```
cmake -S . -B build -G "Ninja" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cd build
cmake --build .
```

Finally, we copy the assets and configuration file to the build directory.

```bash
cmake -E copy ../config.json ./src/config.json
cmake -E copy_directory ../assets ./src/assets
```

Run the game:

```bash
src\wofares_game_engine.exe
```

### Linux build

#### Clone the Repository

```bash
git clone --recursive https://github.com/marleeeeeey/wofares_game_engine.git
```

#### Prerequisites for Building the Project (Linux)

```bash
sudo apt update
sudo apt-get install python3 clang ninja-build curl zip unzip tar autoconf automake libtool python3-pip cmake
pip install jinja2
```

#### Build, run and debug via VSCode tasks (Linux)

- Setup user friendly options via editing file [scripts/vscode_tasks_generator.py](scripts/vscode_tasks_generator.py).
- Run the script [scripts/vscode_tasks_generator.py](scripts/vscode_tasks_generator.py) to generate VSCode tasks with your options.
- Open the project folder in VSCode.
- Run task: `003. Install vcpkg as subfolder`.
- Run task: `050. + Run`.
- For debugging press `F5`.

#### Build, run and debug manually (Linux)

```bash
cd wofares_game_engine
# Branch 2024.06.15 is used because of error on the latest master.
# see: https://github.com/microsoft/vcpkg/issues/40912
git clone --branch 2024.06.15 --single-branch https://github.com/microsoft/vcpkg && ./vcpkg/bootstrap-vcpkg.sh && ./vcpkg/vcpkg install --triplet=x64-linux
cmake -S . -B build -GNinja --preset use_vcpkg -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/debug -- -k 0
cmake -E copy config.json build/debug/src/config.json
cmake -E copy_directory assets build/debug/src/assets
./build/debug/src/wofares_game_engine
```

### Web build

#### Prerequisites for Building the Project (Web)

- Emscripten SDK additionally to [Windows build prerequisites](#prerequisites-for-building-the-project-windows).

#### Build, run and debug via VSCode tasks (Web)

- Set `self.build_for_web = BuildForWeb.YES` and other WebRelated options in [scripts/vscode_tasks_generator.py](scripts/vscode_tasks_generator.py). Example below:

```python

class WebBuildSettings:
    def __init__(self):
        self.build_for_web = BuildForWeb.YES
        self.emsdk_path = "C:/dev/emsdk"
        self.compiler = "emcc"
        self.path_to_ninja = "C:/dev/in_system_path/ninja.exe"  # Fix issue: CMake was unable to find a build program corresponding to "Ninja".  CMAKE_MAKE_PROGRAM is not set.

```

- Run the script [scripts/vscode_tasks_generator.py](scripts/vscode_tasks_generator.py) to generate VSCode tasks with your options.
- Open the project folder in VSCode.
- Run task: `003. Install vcpkg as subfolder`.
- Run task: `010. Configure`.
- Run task: `020. Build`.

#### Build, run and debug manually (Web)

**Install EMSDK**

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
emsdk install latest
emsdk activate latest
# add C:\dev\emsdk\upstream\emscripten\ to the PATH via Environment Variables
```

**Build Game**

```bash
cd wofares_game_engine

git submodule update --init --recursive

# Branch 2024.06.15 is used because of error on the latest master.
# see: https://github.com/microsoft/vcpkg/issues/40912
C:/dev/emsdk/emsdk_env.bat &&  git clone --branch 2024.06.15 --single-branch https://github.com/microsoft/vcpkg && .\\vcpkg\\bootstrap-vcpkg.bat && .\\vcpkg\\vcpkg install --triplet=wasm32-emscripten

C:/dev/emsdk/emsdk_env.bat &&  cmake -S . -B build/debug_web -DCMAKE_BUILD_TYPE=Debug -GNinja -DCMAKE_CXX_COMPILER=clang++ -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=C:/dev/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_MAKE_PROGRAM=C:/dev/in_system_path/ninja.exe -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

C:/dev/emsdk/emsdk_env.bat &&  cmake --build build/debug_web -- -k 0
```

#### Execute the game in the browser

```bash
python -m http.server
```

Open the browser and navigate to `http://localhost:8000/index.html`

Press `F12` to open the developer console and see the game output.

## Change Log

```

*** Start from scratch ***

2024-02-03 1000 Write an OOP wrapper over SDL.
2024-02-04 1030 Separate repository for UTILS.
2024-02-04 1100 Mechanics for scaling level with mouse wheel.
2024-02-05 1230 Mechanics for collisions between cubes in all directions.
2024-02-06 0900 Found a ready set of tilemaps.
2024-02-06 1100 Generate cubes tied to the grid from the tile-map - the base of the level.
2024-02-09 0950 Learn to draw textures on cubes.
2024-02-10 1020 Change the level on the fly to avoid restarting the game because of the editor.
2024-02-11 0800 Divide the level into micro tiles on loading.
2024-02-11 0900 Integrate Box2D for collision calculations.
2024-02-11 1100 Render the angle of inclination of objects from Box2D.
2024-02-11 1215 Make the player movable but subject to physics calculations.
2024-02-11 1330 Add scale between Box2D and SDL.
2024-02-11 1445 Fixed jittering of dynamic tiles clamped between static ones.
2024-02-13 1530 Remove objects that have flown too far away.
2024-02-14 1615 Added InputEventSystem, which helps count the duration of a key press.
2024-02-15 1700 Mechanics of throwing a cube like a grenade (spawn a cube with an angle and applied force).
2024-02-16 0820 Figured out why there are empty lines between tiles (due to fractional scaling).
2024-02-16 0930 Mechanics of aiming at the mouse pointer.
2024-02-16 1045 Added the ability to draw individual tiles by right-clicking the mouse.
2024-02-16 1150 Refactored work with screen, world, and physical world coordinate systems.
2024-02-16 1300 Remove the grenade itself after the explosion.
2024-02-17 1415 Give cubes speed to scatter from the grenade explosion.
2024-02-17 1520 Make a bazooka (included a timer for removing fragments from the map).
2024-02-20 1630 Jump only if there is a cube underfoot.
2024-02-20 1745 Mechanics of smooth return (tracking) of the camera to the moving object.
2024-02-20 1850 Add a background - dark and gloomy.
2024-02-26 2000 Added sounds (voice records for zombie) and music (guitar) to the game.
2024-02-26 2115 Added full screen mode.
2024-02-29 2230 Added filtering of transparent tiles on the map.
2024-02-29 2345 Draw colored pixels instead of figures, this allows taking into account the angle of inclination when rendering.
2024-03-04 0800 Intercept key presses when working in the menu and not send them to the game.
2024-03-04 0915 Calculate the player's capsule in Box2D instead of a rectangle.
2024-03-06 1030 Figured out the technology of Automapping in Tiled.
2024-03-07 1145 Refactored and integrated utils::Config and other utilities.
2024-03-07 1250 Added GameOptions to the game configuration (immediately turned off music by default).
2024-03-07 1350 Indicated background through config.json.
2024-03-08 1450 Support regex or glob for specifying files in config.json.
2024-03-08 1600 Made level destruction like in worms (by pixels).
2024-03-08 1715 Separated program config from assets config.
2024-03-10 1830 Made tag extraction from Aseprite export, and supported Hitbox tag.
2024-03-10 1945 Fix rendering bug of objects with hitboxes.
2024-03-10 2100 Integrated small pixel zombies. Was generated from Aseprite with drafts from ChatGPT.
2024-03-10 2215 Supported randomness in animation using regex expressions for tags in Aseprite.
2024-03-10 2330 Added beautiful exploding particles.
2024-03-13 1015 Define "dead zone" in which the player can move without causing the camera to move.
2024-03-14 1130 Move the player only if there is a cube under him.
2024-03-15 1245 Drop all particles below the level after a specified number of collisions.
2024-03-15 1400 Add shooting with bullets (uzi, pistol).
2024-03-16 1515 Mechanics of switching weapons.
2024-03-16 1630 Added WeaponProps in the code, which allow flexible configuration of weapons.
2024-03-16 1745 Implement TimeToReload for weapons.
2024-03-18 1900 Added AnglePolicy for setting the angle of drawing bullets (VelocityDirection) and player (Fixed).
2024-03-19 2015 Supported updates to Box2D body shapes on the fly during animation change.
2024-03-19 2130 Box2dBodyTuner can now update the body point by point depending on the options provided.
2024-03-20 2245 Started writing naive network code on ASIO directly in the game.
2024-03-24 2355 Compiled network code but realized that I need to look for ready solutions.
2024-03-29 1015 Studied network libraries: studied example of working with SteamNetworkingSockets (Valve).
2024-03-31 1130 Refactored example of working with SteamNetworkingSockets (Valve).
2024-04-03 1245 Integrated SteamNetworkingSockets (Valve) in the game build.
2024-04-04 1400 Studied network libraries: created basic classes for client-server interaction on ASIO.
2024-04-09 1515 Studied network libraries: launched the first version of client-server interaction with handshake on ASIO.
2024-04-10 1630 Drawn static weapon frame.
2024-04-11 1745 Made build under Linux.
2024-04-12 1900 Enabled warnings during the build and fixed several bugs.

*** Start of the LD55 jam ***

2024-04-13 1513 Implement dummy portal catching the player.
2024-04-13 1826 Intergrate evil and portal animations.
2024-04-14 0159 Implement StickyTrap.
2024-04-14 0309 Support DestructionPolicy and ZOrdering: Background, Interiors, Terrain.
2024-04-14 0414 Portal sticky to trap.
2024-04-14 1108 Create fast hybrid explosion fragments.
2024-04-14 1308 Implement portal absorbing fragments.
2024-04-14 1407 Randomize portal speed in time.
2024-04-14 1449 Found a bug with release build during the beta testing (SdCorpse).
2024-04-14 2238 Simplify physic to improve CPU time.
2024-04-14 2316 Fix bug in release with undestructible objects.
2024-04-15 0103 Update collision system. Bullet doesn't collide with Particles.
2024-04-15 0212 Scatter portals if they bump each other.
2024-04-15 1616 Refactor the code. Update documentation with important notes.
2024-04-15 1751 Portal eats player emmediately if it's close enough.
2024-04-15 2338 Fix bug with different speed for players.
2024-04-15 2353 Fix bug when portal searching players incorrectly.
2024-04-16 0105 Portal burst when eats enought. Family member summoing from the portal. All players moving sinchronized.
2024-04-16 0217 Integrate fire animation for bullet and grenade. Remove other weapons. Jump - Space.
2024-04-16 0245 Draw animation for building block.
2024-04-16 0324 One eating counter for all portals. Decrease portal sleep time.
2024-04-16 0411 Create game intruction on start.

*** End of the LD55 jam development. Bug fixing and improvements ***

2024-04-16 1250 Fix start menu bug.
2024-04-16 1317 Remove background from the game.
2024-04-23 1300 Prepare build for web (WASM Emscripten).
2024-04-23 1400 Remove music fade-in/fade-out.
2024-04-26 0215 Add maps from easy to dificult level.
2024-04-27 0228 Improve sound system. Add several sounds (fire, explosion, eating).
2024-04-29 1300 Add layer with indestructible tiles.
2024-04-29 1400 Fix bug with control buttons sticking when reloading the level.
2024-04-29 1500 Improve player jump. Now it depends on the duration of pressing the key.
2024-04-29 1600 Add debug functions for drawing sensors and bounding boxes.
2024-04-29 1700 Enable VSync.
2024-04-30 1800 Disable bullet collisions with the player.
2024-05-01 1900 Major improvement of debugging tools: added the ability to track objects.
2024-05-01 2000 Fix bugs in the explosion mechanics.
2024-05-01 2100 Expand the building block.

*** New iteration of the game development ***

2024-05-02 0100 Update repository with latest changes from LD55.

```
