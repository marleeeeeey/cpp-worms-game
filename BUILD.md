# Build instructions

- [Windows build](#windows-build)
  - [Clone the Repository](#clone-the-repository)
  - [Prerequisites for Building the Project (Windows)](#prerequisites-for-building-the-project-windows)
  - [Build, run and debug via VSCode tasks (Windows)](#build-run-and-debug-via-vscode-tasks-windows)
  - [Build, run and debug manually (Windows)](#build-run-and-debug-manually-windows)
- [Linux build](#linux-build)
  - [Clone the Repository](#clone-the-repository-1)
  - [Prerequisites for Building the Project (Linux)](#prerequisites-for-building-the-project-linux)
  - [Build, run and debug via VSCode tasks (Linux)](#build-run-and-debug-via-vscode-tasks-linux)
  - [Build, run and debug manually (Linux)](#build-run-and-debug-manually-linux)
- [Web build](#web-build)
  - [Prerequisites for Building the Project (Web)](#prerequisites-for-building-the-project-web)
  - [Build, run and debug via VSCode tasks (Web)](#build-run-and-debug-via-vscode-tasks-web)
  - [Build, run and debug manually (Web)](#build-run-and-debug-manually-web)
  - [Execute the game in the browser](#execute-the-game-in-the-browser)

Wofares Game Engine has been developed, built, and tested on **Windows** and **Web(Emscripten)**. Build also works on **Linux(Ubuntu)** but not tested yet.

## Windows build

### Clone the Repository

```bash
git clone --recursive https://github.com/marleeeeeey/wofares-game-engine.git
```

### Prerequisites for Building the Project (Windows)

- CMake.
- Ninja.
- Clang compiler (other compilers may work, but are not officially supported).
- Git (to load submodules).
- Visual Studio Code (recommended for development).
  - Clangd extension (recommended for code analysis).
- Python and 7zip (for packaging game to achive).

### Build, run and debug via VSCode tasks (Windows)

- Setup user friendly options via editing file [scripts/vscode_tasks_generator.py](scripts/vscode_tasks_generator.py).
- Run the script [scripts/vscode_tasks_generator.py](scripts/vscode_tasks_generator.py) to generate VSCode tasks with your options.
- Open the project folder in VSCode.
- Run task: `003. Install vcpkg as subfolder`.
- Run task: `050. + Run`.
- For debugging press `F5`.

### Build, run and debug manually (Windows)

To build Wofares Game Engine on Windows, it's recommended to obtain the dependencies by using vcpkg. The following instructions assume that you will follow the vcpkg recommendations and install vcpkg as a subfolder. If you want to use "classic mode" or install vcpkg somewhere else, you're on your own.

This project define it's dependences:
1. In a `vcpkg.json` file, and you are pulling in vcpkg's cmake toolchain file.
2. As git submodules in the `thirdparty` directory. Because some of the libraries not available in vcpkg or have an error in the vcpkg port file.

First, we bootstrap a project-specific installation of vcpkg ("manifest mode") in the default location, `<project root>/vcpkg`. From the project root, run these commands:

```bash
cd wofares_game_engine
 Branch 2024.06.15 is used because of error on the latest master.
 see: https://github.com/microsoft/vcpkg/issues/40912
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

## Linux build

### Clone the Repository

```bash
git clone --recursive https://github.com/marleeeeeey/wofares_game_engine.git
```

### Prerequisites for Building the Project (Linux)

```bash
sudo apt update
sudo apt-get install python3 clang ninja-build curl zip unzip tar autoconf automake libtool python3-pip cmake
pip install jinja2
```

### Build, run and debug via VSCode tasks (Linux)

- Setup user friendly options via editing file [scripts/vscode_tasks_generator.py](scripts/vscode_tasks_generator.py).
- Run the script [scripts/vscode_tasks_generator.py](scripts/vscode_tasks_generator.py) to generate VSCode tasks with your options.
- Open the project folder in VSCode.
- Run task: `003. Install vcpkg as subfolder`.
- Run task: `050. + Run`.
- For debugging press `F5`.

### Build, run and debug manually (Linux)

```bash
cd wofares_game_engine
 Branch 2024.06.15 is used because of error on the latest master.
 see: https://github.com/microsoft/vcpkg/issues/40912
git clone --branch 2024.06.15 --single-branch https://github.com/microsoft/vcpkg && ./vcpkg/bootstrap-vcpkg.sh && ./vcpkg/vcpkg install --triplet=x64-linux
cmake -S . -B build -GNinja --preset use_vcpkg -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/debug -- -k 0
cmake -E copy config.json build/debug/src/config.json
cmake -E copy_directory assets build/debug/src/assets
./build/debug/src/wofares_game_engine
```

## Web build

### Prerequisites for Building the Project (Web)

- Emscripten SDK additionally to [Windows build prerequisites](#prerequisites-for-building-the-project-windows).

### Build, run and debug via VSCode tasks (Web)

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

### Build, run and debug manually (Web)

**Install EMSDK**

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
emsdk install latest
emsdk activate latest
 add C:\dev\emsdk\upstream\emscripten\ to the PATH via Environment Variables
```

**Build Game**

```bash
cd wofares_game_engine

git submodule update --init --recursive

 Branch 2024.06.15 is used because of error on the latest master.
  see: https://github.com/microsoft/vcpkg/issues/40912
C:/dev/emsdk/emsdk_env.bat &&  git clone --branch 2024.06.15 --single-branch https://github.com/microsoft/vcpkg && .\\vcpkg\\bootstrap-vcpkg.bat && .\\vcpkg\\vcpkg install --triplet=wasm32-emscripten

C:/dev/emsdk/emsdk_env.bat &&  cmake -S . -B build/debug_web -DCMAKE_BUILD_TYPE=Debug -GNinja -DCMAKE_CXX_COMPILER=clang++ -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=C:/dev/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_MAKE_PROGRAM=C:/dev/in_system_path/ninja.exe -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

C:/dev/emsdk/emsdk_env.bat &&  cmake --build build/debug_web -- -k 0
```

### Execute the game in the browser

```bash
python -m http.server
```

Open the browser and navigate to `http://localhost:8000/index.html`

Press `F12` to open the developer console and see the game output.
