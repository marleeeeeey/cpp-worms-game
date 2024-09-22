#pragma once
#include "utils/factories/base_objects_factory.h"
#include <entt/entt.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/game_objects_factory.h>
#include <utils/level_info.h>
#include <utils/resources/resource_manager.h>
#include <utils/sdl/sdl_RAII.h>
#include <utils/systems/box2d_entt_contact_listener.h>

class MapLoaderSystem
{
    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    ResourceManager& resourceManager;
    Box2dEnttContactListener& contactListener;
    GameOptions& gameState;
    GameObjectsFactory& gameObjectsFactory;
    BaseObjectsFactory& baseObjectsFactory;
    CoordinatesTransformer coordinatesTransformer;
    int tileWidth;
    int tileHeight;
    int colAndRowNumber;
    int miniWidth;
    int miniHeight;
    size_t createdTiles = 0;
    size_t invisibleTilesNumber = 0;
    std::shared_ptr<SDLTextureRAII> tilesetTexture;
    std::shared_ptr<SDLSurfaceRAII> tilesetSurface; // Optional. Used when Streaming access is needed.
    LevelInfo currentLevelInfo;
public:
    MapLoaderSystem(
        EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager, Box2dEnttContactListener& contactListener, GameObjectsFactory& gameObjectsFactory,
        BaseObjectsFactory& baseObjectsFactory);
    void LoadMap(const LevelInfo& levelInfo);
private:
    void ParseTileLayer(const nlohmann::json& layer, SpawnTileOption tileOptions);
    void ParseObjectLayer(const nlohmann::json& layer);
    void CalculateLevelBoundsWithBufferZone();
    void ParseTile(int tileId, int layerCol, int layerRow, SpawnTileOption tileOptions);
private: // Low level functions.
    std::filesystem::path ReadPathToTileset(const nlohmann::json& mapJson);
    void RecreateBox2dWorld();
};
