# Recursively collect all .cpp files from the current source directory and subdirectories.
file(GLOB_RECURSE wofares_game_engine_SOURCES "*.cpp")

# Create an executable with the collected source files.
add_executable(wofares_game_engine ${wofares_game_engine_SOURCES})

if(EMSCRIPTEN)
    # To copy the assets and config.json to the WASM's virtual filesystem.
    set(EM_LINK_FLAGS "--preload-file \"${CMAKE_SOURCE_DIR}/assets\"@/assets")
    set(EM_LINK_FLAGS "${EM_LINK_FLAGS} --preload-file \"${CMAKE_SOURCE_DIR}/config.json\"@/config.json")

    # To increase memory limit for the game.
    set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -s INITIAL_MEMORY=33554432")
    set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
    set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -s ABORTING_MALLOC=0")

    # Set the flags.
    set_target_properties(wofares_game_engine PROPERTIES LINK_FLAGS "${EM_LINK_FLAGS}")

    # Enable the HTML output.
    set_target_properties(wofares_game_engine PROPERTIES SUFFIX ".html")
endif()

target_compile_definitions(wofares_game_engine
    PRIVATE

    # MY_DEBUG # TODO5: Uncomment this to enable extra debug mode.
)

target_compile_options(wofares_game_engine PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wpedantic

    # ################ Some warnings from HolyBlackCat's code. ################
    -Wdeprecated # search for deprecated functions.
    -Wextra-semi # search for unused `;` in the code.
    -Wimplicit-fallthrough # search for missing `break` and `cases` in switch cases.

    # -Wconversion # search for implicit conversions. Hard to fix everything if not applying from scratch.

    # ################### Disable some warnings temporarily. ##################
    $<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-declarations> # TODO4. Remove this. glob/glob.hpp:173:28: warning: 'getenv' is deprecated
)

target_link_libraries(wofares_game_engine
    PRIVATE

    # vcpkg build libraries:
    box2d::box2d
    EnTT::EnTT

    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
    SDL2::SDL2_gfx

    # custom build libraries:
    imgui # Because of this package unavailability in linux package manager.
    my_cpp_utils
)

target_include_directories(wofares_game_engine
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/glob/single_include
)

# copy assets
add_custom_command(TARGET wofares_game_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:wofares_game_engine>/assets")

# copy config.json
add_custom_command(TARGET wofares_game_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/config.json"
    "$<TARGET_FILE_DIR:wofares_game_engine>/config.json")