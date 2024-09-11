import json
from enum import Enum, auto
import os
import sys

##################################### SETTINGS #####################################


class WebBuildSettings:
    def __init__(self):
        self.build_for_web = BuildForWeb.NO
        self.emsdk_path = "C:/dev/emsdk"
        self.compiler = "emcc"
        self.path_to_ninja = "C:/dev/in_system_path/ninja.exe"  # Fix issue: CMake was unable to find a build program corresponding to "Ninja". CMAKE_MAKE_PROGRAM is not set.

        if self.build_for_web == BuildForWeb.YES:
            if not os.path.isdir(self.emsdk_path):
                raise FileNotFoundError(f"Directory {self.emsdk_path} not found.")
            if not os.path.isfile(self.path_to_ninja):
                raise FileNotFoundError(f"File {self.path_to_ninja} not found.")


class Settings:
    def __init__(self):
        self.platform = Platform.WINDOWS
        self.build_type = BuildType.DEBUG
        self.compiler = "clang++"
        self.make_tool = "Ninja"
        self.stop_on_first_error = StopOnFirstError.YES
        self.export_compile_commands = ExportCompileCommands.YES
        self.executable_name = "wofares_game_engine"
        self.web = WebBuildSettings()

    def compiler_name(self):
        if self.web.build_for_web == BuildForWeb.YES:
            return self.web.compiler
        return self.compiler

    def build_folder(self):
        suffix = {
            BuildForWeb.YES: "_web",
            BuildForWeb.NO: "",
        }[self.web.build_for_web]

        return {
            BuildType.DEBUG: f"build/Debug{suffix}",
            BuildType.RELEASE: f"build/Release{suffix}",
        }[self.build_type]

    def build_type_name(self):
        return {
            BuildType.DEBUG: "Debug",
            BuildType.RELEASE: "Release",
        }[self.build_type]

    def path_to_python(self):
        return {
            Platform.WINDOWS: "python",
            Platform.LINUX: "python3",
        }[self.platform]

    def path_to_7z(self):
        return {
            Platform.WINDOWS: "C:/Program Files/7-Zip/7z.exe",
            Platform.LINUX: "7z",
        }[self.platform]

    def vcpkg_extra_args(self):
        if self.web.build_for_web == BuildForWeb.YES:
            return f"-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE={self.web.emsdk_path}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_MAKE_PROGRAM={self.web.path_to_ninja}"
        return ""

    def triplet(self):
        if self.web.build_for_web == BuildForWeb.YES:
            return "wasm32-emscripten"

        result = {
            Platform.WINDOWS: "x64-Windows",
            Platform.LINUX: "x64-linux",
        }[self.platform]

        return result

    def setup_env(self):
        if self.web.build_for_web == BuildForWeb.NO:
            return ""
        return {
            Platform.WINDOWS: f"{self.web.emsdk_path}/emsdk_env.bat && ",
            Platform.LINUX: f"source {self.web.emsdk_path}/emsdk_env.sh && ",
        }[self.platform]


####################################### COMMON TYPES #######################################


class Platform(Enum):
    WINDOWS = auto()
    LINUX = auto()


class BuildType(Enum):
    DEBUG = auto()
    RELEASE = auto()


class StopOnFirstError(Enum):
    YES = auto()
    NO = auto()


class ExportCompileCommands(Enum):
    YES = auto()
    NO = auto()


class BuildForWeb(Enum):
    YES = auto()
    NO = auto()


##################################### COMMON FUNCTIONS #####################################


def common_append_vscode_statusbar_label(task, statusbar_label: str = None):
    if statusbar_label == None:
        return task

    if statusbar_label == "":
        return task

    task["options"] = {
        "statusbar": {
            "hide": False,
            "label": statusbar_label,
        }
    }

    return task


def common_set_type_shell(task):
    task["type"] = "shell"
    return task


def get_build_alias_from_settings(s: Settings):
    build_alias_matching = {
        (Platform.WINDOWS, BuildType.DEBUG, BuildForWeb.NO): "debug",
        (Platform.WINDOWS, BuildType.RELEASE, BuildForWeb.NO): "release",
        (Platform.WINDOWS, BuildType.RELEASE, BuildForWeb.YES): "web",
    }

    current_alias = None
    if (s.platform, s.build_type, s.web.build_for_web) in build_alias_matching:
        current_alias = build_alias_matching[(s.platform, s.build_type, s.web.build_for_web)]
    else:
        current_alias = "custom"

    return current_alias


def get_next_build_alias(current_alias: str):
    return {
        "custom": "debug",
        "debug": "release",
        "release": "web",
        "web": "debug",
    }[current_alias]


##################################### SPECIFIC TASKS #####################################


def generate_000_switch_build_alias_task(s: Settings):
    """
    This task is used to switch between different build aliases in the loop.
    """

    # Replace current alias with upper case.
    current_alias = get_build_alias_from_settings(s)
    label = "000. custom->[debug->release->web]"
    label = label.replace(current_alias, current_alias.upper())
    next_build_alias = get_next_build_alias(current_alias)

    return {
        "label": label,
        "command": f"{s.path_to_python()} scripts/vscode_tasks_generator.py {next_build_alias}",
    }


def generate_001_task_with_config_name(s: Settings):

    config_name = {
        Platform.WINDOWS: "WIN",
        Platform.LINUX: "LINUX",
    }[s.platform]

    config_name += {
        BuildType.DEBUG: "_DEBUG",
        BuildType.RELEASE: "_RELEASE",
    }[s.build_type]

    config_name += {
        BuildForWeb.YES: "_WEB",
        BuildForWeb.NO: "",
    }[s.web.build_for_web]

    build_alias = get_build_alias_from_settings(s)

    statusbar_label = f"Config: {config_name}"

    status_bar_options = {
        "statusbar": {
            "hide": False,
            "label": statusbar_label,
        }
    }

    return {
        "label": f"001. {statusbar_label}",
        "command": f"{s.path_to_python()} scripts/vscode_tasks_generator.py {build_alias}",
        "options": status_bar_options,
    }


def generate_002_remove_vcpkg_folders_task(s: Settings):
    return {
        "label": "002. Remove vcpkg folders",
        "command": f"cmake -E remove_directory vcpkg && cmake -E remove_directory vcpkg_installed",
    }


def generate_003_remove_build_folder_task(s: Settings):
    return {
        "label": f"003. Remove build folder",
        "command": f"cmake -E remove_directory {s.build_folder()}",
    }


def generate_005_git_submodule_update_task(s: Settings):
    return {
        "label": "005. Git submodule update",
        "command": "git submodule update --init --recursive",
    }


def generate_007_install_vcpkg_as_subfolder_task(s: Settings):
    command = {
        Platform.WINDOWS: f"{s.setup_env()} git clone --branch 2024.06.15 --single-branch https://github.com/microsoft/vcpkg && .\\vcpkg\\bootstrap-vcpkg.bat && .\\vcpkg\\vcpkg install --triplet={s.triplet()}",
        Platform.LINUX: f"{s.setup_env()} git clone --branch 2024.06.15 --single-branch https://github.com/microsoft/vcpkg && ./vcpkg/bootstrap-vcpkg.sh && ./vcpkg/vcpkg install --triplet={s.triplet()}",
    }[s.platform]

    return {
        "label": "007. Install vcpkg as subfolder",
        "command": command,
    }


def generate_010_cmake_configure_task(s: Settings):
    export_compile_commands = {
        ExportCompileCommands.YES: " -DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        ExportCompileCommands.NO: "",
    }[s.export_compile_commands]

    command = (
        f" {s.setup_env()} cmake -S . -B {s.build_folder()}"
        f" --preset use_vcpkg"
        f" -DCMAKE_BUILD_TYPE={s.build_type_name()}"
        f" -G{s.make_tool} -DCMAKE_CXX_COMPILER={s.compiler}"
        f" {s.vcpkg_extra_args()} {export_compile_commands}"
        f" && cmake -E copy {s.build_folder()}/compile_commands.json build/compile_commands.json",  # compile_commands.json is visible from `build` folder only
    )

    depends_on = {
        # For emscripten build double configure leads to error.
        # That's why we add dependecy to remove build folder before configure.
        BuildForWeb.YES: ["003. Remove build folder"],
        BuildForWeb.NO: [],
    }[s.web.build_for_web]

    return {
        "label": "010. (+) Configure",
        "command": command,
        "dependsOn": depends_on,
    }


def generate_020_cmake_build_task(s: Settings):
    stop_on_first_error = {
        StopOnFirstError.YES: "-k 0",
        StopOnFirstError.NO: "",
    }[s.stop_on_first_error]

    extra_command = {
        BuildForWeb.YES: f" && {s.path_to_python()} scripts/rename_to_index_html.py ${{workspaceFolder}} {s.build_folder()}",
        BuildForWeb.NO: "",
    }[s.web.build_for_web]

    return {
        "label": "020. + Build",
        "command": f"{s.setup_env()} cmake --build {s.build_folder()} -- {stop_on_first_error} {extra_command}",
        "dependsOn": ["010. (+) Configure"],
    }


def generate_030_copy_config_json_task(s: Settings):
    return {
        "label": "030. + Copy config.json",
        "command": f"cmake -E copy config.json {s.build_folder()}/src/config.json",
        "dependsOn": ["020. + Build"],
    }


def generate_040_copy_assets_task(s: Settings):

    # Remove all files in target folder before copying.
    remove_command = f"cmake -E remove_directory {s.build_folder()}/src/assets"
    # Copy all files from source folder to target folder.
    copy_command = f"cmake -E copy_directory assets {s.build_folder()}/src/assets"

    return {
        "label": "040. + Copy assets",
        "command": f"{remove_command} && {copy_command}",
        "dependsOn": ["030. + Copy config.json"],
    }


def generate_050_run_task(s: Settings):
    return {
        "label": "050. + Run",
        "command": f"${{workspaceFolder}}/{s.build_folder()}/src/{s.executable_name}.exe",
        "dependsOn": ["040. + Copy assets"],
    }


def generate_060_pack_task(s: Settings):
    web_or_desktop = {
        BuildForWeb.YES: "web",
        BuildForWeb.NO: "desktop",
    }[s.web.build_for_web]

    depends_on = {
        BuildForWeb.YES: ["020. + Build"],
        BuildForWeb.NO: ["040. + Copy assets"],
    }[s.web.build_for_web]

    return {
        "label": "060. + Pack",
        "command": f'{s.path_to_python()} scripts/pack_binaries.py "{s.path_to_7z()}" ${{workspaceFolder}} {s.build_folder()} {s.executable_name}_{s.build_type_name()} {web_or_desktop}',
        "dependsOn": depends_on,
    }


def generate_070_just_run_task(s: Settings):
    return {
        "label": "070. Run",
        "command": f"${{workspaceFolder}}/build/debug/src/{s.executable_name}.exe",
    }


def generate_080_web_run_server_task(s: Settings):
    return {
        "label": "080. Run web server",
        "command": f"{s.path_to_python()} -m http.server 8000 --directory {s.build_folder()}/src",
    }


def generate_090_open_game_link_task(s: Settings):
    return {
        "label": "090. Open game link",
        "command": "start http://localhost:8000/index.html",
    }


##################################### MAIN FUNCTION #####################################


def generate_tasks(platform: Platform = None, build_type: BuildType = None, build_for_web: BuildForWeb = None):
    tasks = {"version": "2.0.0", "tasks": []}
    settings = Settings()

    # Override settings if arguments are provided
    if platform:
        settings.platform = platform
    if build_type:
        settings.build_type = build_type
    if build_for_web:
        settings.web.build_for_web = build_for_web

    functions_and_statusbar_name = [
        (generate_000_switch_build_alias_task, None),
        (generate_001_task_with_config_name, ""),
        (generate_002_remove_vcpkg_folders_task, None),
        (generate_003_remove_build_folder_task, None),
        (generate_005_git_submodule_update_task, None),
        (generate_007_install_vcpkg_as_subfolder_task, None),
        (generate_010_cmake_configure_task, "Configure"),
        (generate_020_cmake_build_task, "Build"),
        (generate_060_pack_task, "Pack"),
    ]

    if settings.web.build_for_web == BuildForWeb.NO:
        functions_and_statusbar_name += [
            (generate_030_copy_config_json_task, None),
            (generate_040_copy_assets_task, None),
            (generate_050_run_task, "Run"),
            (generate_070_just_run_task, None),
        ]

    if settings.web.build_for_web == BuildForWeb.YES:
        functions_and_statusbar_name += [
            (generate_080_web_run_server_task, "RunWebServer"),
            (generate_090_open_game_link_task, "OpenGameLink"),
        ]

    for function, statusbar_name in functions_and_statusbar_name:
        task = function(settings)
        task = common_append_vscode_statusbar_label(task, statusbar_name)
        task = common_set_type_shell(task)
        tasks["tasks"].append(task)

    return tasks


if __name__ == "__main__":
    if len(sys.argv) > 2:
        print(R"Usage: scripts\vscode_tasks_generator.py <release/debug/web>")
        sys.exit(1)

    platform: Platform = None
    build_type: BuildType = None
    build_for_web: BuildForWeb = None

    if len(sys.argv) == 2:
        arg = sys.argv[1]
        if arg == "release":
            build_type = BuildType.RELEASE
            platform = Platform.WINDOWS
            build_for_web = BuildForWeb.NO
        elif arg == "debug":
            build_type = BuildType.DEBUG
            platform = Platform.WINDOWS
            build_for_web = BuildForWeb.NO
        elif arg == "web":
            build_type = BuildType.RELEASE
            platform = Platform.WINDOWS
            build_for_web = BuildForWeb.YES
        else:
            print(R"Usage: scripts\vscode_tasks_generator.py <release/debug/web>")
            sys.exit(1)

    current_file_dir = os.path.dirname(os.path.abspath(__file__))
    tasks_config = generate_tasks(platform=platform, build_type=build_type, build_for_web=build_for_web)
    with open(f"{current_file_dir}/../.vscode/tasks.json", "w") as tasks_file:
        json.dump(tasks_config, tasks_file, indent=4)
