cmake_minimum_required(VERSION 3.15)
project(wofares_game_engine VERSION 1.0)

# #################### Setup compiler options #######################
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generate compile_commands.json.

# ############### Searching some packages in system #################
find_package(box2d CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(sdl2-gfx CONFIG REQUIRED)

# ####################### Add subdirectories ########################
add_subdirectory(thirdparty/my_cpp_utils)
add_subdirectory(src)

# ################# Build imgui from submodules #####################
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui)

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

target_include_directories(imgui
    PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(imgui
    PRIVATE
    SDL2::SDL2
)
