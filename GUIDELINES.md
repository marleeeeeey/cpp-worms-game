# Guidelines

- [Development Guidelines](#development-guidelines)
  - [Efficient Component Access (Entt library)](#efficient-component-access-entt-library)
  - [Object Factory](#object-factory)
  - [Component Naming Conventions](#component-naming-conventions)
  - [Coordinate Systems and Object Sizing](#coordinate-systems-and-object-sizing)
  - [C++ Style Guide](#c-style-guide)
  - [Configuration Style Guide](#configuration-style-guide)
  - [Animation Editor and Engine Integration](#animation-editor-and-engine-integration)
- [Assets Editing Guidelines](#assets-editing-guidelines)
  - [Aseprite Guidelines](#aseprite-guidelines)
  - [Tiled Map Editor Guidelines](#tiled-map-editor-guidelines)
- [Comment guidelines](#comment-guidelines)

## Development Guidelines

### Efficient Component Access (Entt library)

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

### Object Factory

- **ObjectsFactory** is the only object that should add new components to the `entt::registry`.
  - Any method that creates a new object must start with `Spawn`.

### Component Naming Conventions

- Class names for components must end with `Component`.
  - Header files containing components should end with `_components.h`.

### Coordinate Systems and Object Sizing

- Texture sizes are measured in pixels, while the sizes of objects in the physical world are measured in meters.
- There are three coordinate systems: screen, pixel world, and physical world. Variable names should end with `Screen`, `World` (TODO3: rename to `Pixels`), or `Physics`.

### C++ Style Guide

- Prefer using `enum class` over boolean variables.
- Favor composition over inheritance.
- Use descriptive and clear naming conventions that reflect the purpose and usage of the variable or function.

### Configuration Style Guide

- In `config.json`, section names should correspond to the class names that use them.
- Variable naming in `config.json` should be a direct copy of the variable names in the code.
- Debug options should start with `debug` and may be located in every section.

### Animation Editor and Engine Integration

- The player animation stores the BBox (bounding box) of the object used for collision detection.
- Bullet animations should not consider the dimensions of the BBox for speed calculations.

## Assets Editing Guidelines

### Aseprite Guidelines

- **Tagging**: Ensure that all frames are tagged, as these tags are referenced in the C++ code for animation handling.
- **Hitbox Frame**: Include a specifically named frame `Hitbox` within your Aseprite files, which will be used to define interaction boundaries in the game.

### Tiled Map Editor Guidelines

- **Layer Structure**: Your maps should be organized with the following layers from bottom to top:
  - `background`: For non-interactive scenery that appears behind all game entities.
  - `interiors`: For elements like indoor furnishings that players can potentially interact with but are not part of the terrain.
  - `terrain`: For the main walkable and interactive layer of the game environment.
  - `objects`: For movable or interactive objects that players can interact with during the game.

## Comment guidelines

- **Code Block Comments**: Surround the comments for a block of code with repeating // or # symbols depending on the programming language. This practice helps in maintaining visibility of the block's purpose when code folding is enabled in the editor.

```cmake
 #################### Setup compiler options #######################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generate compile_commands.json.
```

```cpp
public: ///////////////////////////////////////// Damage. /////////////////////////////////////////
    size_t damageRadiusWorld = 10; // Radius of the damage in pixels.
    float damageForce = 0.5; // Force of the damage.
public: ///////////////////////////////////////// Ammo. ///////////////////////////////////////////
    size_t ammoInStorage = 100; // Current number of bullets except in the clip.
    size_t ammoInClip = 10; // Current number of bullets in the clip.
    size_t clipSize = 10; // Max number of bullets in the clip.
```
