#include "map_loader_system.h"
#include "utils/factories/base_objects_factory.h"
#include <SDL_image.h>
#include <box2d/b2_math.h>
#include <ecs/components/physics_components.h>
#include <fstream>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/math_utils.h>
#include <utils/box2d/box2d_glm_operators.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/logger.h>
#include <utils/math_utils.h>
#include <utils/sdl/sdl_texture_process.h>

MapLoaderSystem::MapLoaderSystem(
    EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager, Box2dEnttContactListener& contactListener, GameObjectsFactory& gameObjectsFactory,
    BaseObjectsFactory& baseObjectsFactory)
  : registryWrapper(registryWrapper), registry(registryWrapper), resourceManager(resourceManager), contactListener(contactListener),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), gameObjectsFactory(gameObjectsFactory), baseObjectsFactory(baseObjectsFactory),
    coordinatesTransformer(registry)
{}

void MapLoaderSystem::LoadMap(const LevelInfo& levelInfo)
{
    RecreateBox2dWorld();

    currentLevelInfo = levelInfo;

    // Save map file path and load it as json.
    std::ifstream file(levelInfo.tiledMapPath);
    if (!file.is_open())
        throw std::runtime_error("Failed to open map file");

    nlohmann::json mapJson;
    file >> mapJson;

    // Load tileset texture and surface. Surface is used to search for invisible tiles.
    std::filesystem::path tilesetPath = ReadPathToTileset(mapJson);
    tilesetTexture = resourceManager.GetTexture(tilesetPath);
    tilesetSurface = resourceManager.GetSurface(tilesetPath);

    // Load background texture.
    gameState.levelOptions.backgroundInfo.texture = resourceManager.GetTexture(levelInfo.backgroundPath);

    // Assume all tiles are of the same size.
    tileWidth = mapJson["tilewidth"];
    tileHeight = mapJson["tileheight"];

    // Calculate mini tile size: 4x4 mini tiles in one big tile.
    colAndRowNumber = utils::GetConfig<size_t, "MapLoaderSystem.tileSplitFactor">();
    miniWidth = tileWidth / colAndRowNumber;
    miniHeight = tileHeight / colAndRowNumber;

    // Iterate over each tile layer.
    for (const auto& layer : mapJson["layers"])
    {
        if (layer["type"] == "tilelayer")
        {
            if (layer["name"] == "background")
                ParseTileLayer(layer, {SpawnTileOption::CollidableOption::Transparent, SpawnTileOption::DesctructibleOption::Indestructible, ZOrderingType::Background});
            if (layer["name"] == "interiors")
                ParseTileLayer(layer, {SpawnTileOption::CollidableOption::Transparent, SpawnTileOption::DesctructibleOption::Indestructible, ZOrderingType::Interiors});
            if (layer["name"] == "terrain")
                ParseTileLayer(layer, {SpawnTileOption::CollidableOption::Collidable, SpawnTileOption::DesctructibleOption::Destructible, ZOrderingType::Terrain});
            if (layer["name"] == "terrain_no_destructible")
                ParseTileLayer(layer, {SpawnTileOption::CollidableOption::Collidable, SpawnTileOption::DesctructibleOption::Indestructible, ZOrderingType::Terrain});
        }
        else if (layer["type"] == "objectgroup")
        {
            ParseObjectLayer(layer);
        }
    }

    CalculateLevelBoundsWithBufferZone();

    // Log warnings.
    if (invisibleTilesNumber > 0)
        MY_LOG(info, "There are {}/{} tiles with invisible pixels", invisibleTilesNumber, createdTiles);
    if (createdTiles == 0)
    {
        MY_LOG(warn, "No tiles were created during map loading {}", levelInfo.tiledMapPath.string());
        if (invisibleTilesNumber > 0)
            MY_LOG(warn, "All tiles are invisible");
    }
}

void MapLoaderSystem::ParseTileLayer(const nlohmann::json& layer, SpawnTileOption tileOptions)
{
    auto physicsWorld = gameState.physicsWorld;

    int layerCols = layer["width"];
    int layerRows = layer["height"];
    const auto& tiles = layer["data"];

    // Create entities for each tile.
    for (int layerRow = 0; layerRow < layerRows; ++layerRow)
    {
        for (int layerCol = 0; layerCol < layerCols; ++layerCol)
        {
            int tileId = tiles[layerCol + layerRow * layerCols];

            // Skip empty tiles.
            if (tileId <= 0)
                continue;

            ParseTile(tileId, layerCol, layerRow, tileOptions);
        }
    }
}

void MapLoaderSystem::ParseObjectLayer(const nlohmann::json& layer)
{
    for (const auto& object : layer["objects"])
    {
        if (object["type"] == "Player")
        {
            std::string objectName = object["name"];
            auto posWorld = glm::vec2(object["x"], object["y"]);
            gameObjectsFactory.SpawnPlayer(posWorld, objectName);
        }

        if (object["type"] == "Portal")
        {
            std::string objectName = object["name"];
            auto posWorld = glm::vec2(object["x"], object["y"]);
            gameObjectsFactory.SpawnPortal(posWorld, objectName);
        }

        if (object["type"] == "Turret")
        {
            std::string objectName = object["name"];
            auto posWorld = glm::vec2(object["x"], object["y"]);
            gameObjectsFactory.SpawnTurret(posWorld, objectName);
        }
    }
}

void MapLoaderSystem::CalculateLevelBoundsWithBufferZone()
{
    auto& lb = gameState.levelOptions.levelBox2dBounds;
    auto& bz = gameState.levelOptions.bufferZone;
    MY_LOG(debug, "Level bounds: min: ({}, {}), max: ({}, {})", lb.min.x, lb.min.y, lb.max.x, lb.max.y);
    lb.min -= bz;
    lb.max += bz;
    MY_LOG(debug, "Level bounds with buffer zone: min: ({}, {}), max: ({}, {})", lb.min.x, lb.min.y, lb.max.x, lb.max.y);
}

void MapLoaderSystem::ParseTile(int tileId, int layerCol, int layerRow, SpawnTileOption tileOptions)
{
    auto physicsWorld = gameState.physicsWorld;

    SDL_Rect textureSrcRect = CalculateSrcRect(tileId, tileWidth, tileHeight, tilesetTexture);

    Box2dBodyCreator box2dBodyCreator(registry);

    // Create entities for each mini tile inside the tile.
    for (int miniRow = 0; miniRow < colAndRowNumber; ++miniRow)
    {
        for (int miniCol = 0; miniCol < colAndRowNumber; ++miniCol)
        {
            SDL_Rect miniTextureSrcRect{textureSrcRect.x + miniCol * miniWidth, textureSrcRect.y + miniRow * miniHeight, miniWidth, miniHeight};

            // Skip invisible tiles.
            {
                if (!tilesetSurface)
                    throw std::runtime_error("tilesetSurface is nullptr");

                if (IsTileInvisible(tilesetSurface->get(), miniTextureSrcRect))
                {
                    invisibleTilesNumber++;
                    continue;
                }
            }

            // Create tile entity.
            float miniTileWorldPositionX = layerCol * tileWidth + miniCol * miniWidth;
            float miniTileWorldPositionY = layerRow * tileHeight + miniRow * miniHeight;
            glm::vec2 miniTileWorldPosition{miniTileWorldPositionX, miniTileWorldPositionY};
            auto textureRect = TextureRect{tilesetTexture, miniTextureSrcRect};
            auto tileEntity = baseObjectsFactory.SpawnTile(miniTileWorldPosition, miniWidth, textureRect, tileOptions);

            // Update level bounds.
            auto bodyRAII = registry.get<PhysicsComponent>(tileEntity).bodyRAII;
            const b2Vec2& bodyPosition = bodyRAII->GetBody()->GetPosition();
            auto& levelBounds = gameState.levelOptions.levelBox2dBounds;
            levelBounds.min = utils::Vec2Min(levelBounds.min, bodyPosition);
            levelBounds.max = utils::Vec2Max(levelBounds.max, bodyPosition);

            createdTiles++;
        }
    }
}

std::filesystem::path MapLoaderSystem::ReadPathToTileset(const nlohmann::json& mapJson)
{
    std::filesystem::path tilesetPath;

    if (mapJson.contains("tilesets") && mapJson["tilesets"][0].contains("source"))
    {
        // if map contains path to tileset.json
        //  "tilesets":[
        //         {
        //          "firstgid":1,
        //          "source":"tileset.json"
        //         }]
        std::filesystem::path tilesetJsonPath = currentLevelInfo.tiledMapPath.parent_path() / mapJson["tilesets"][0]["source"].get<std::string>();
        std::ifstream tilesetFile(tilesetJsonPath);
        if (!tilesetFile.is_open())
            throw std::runtime_error("Failed to open tileset file");

        nlohmann::json tilesetJson;
        tilesetFile >> tilesetJson;
        tilesetPath = currentLevelInfo.tiledMapPath.parent_path() / tilesetJson["image"].get<std::string>();
    }
    else if (mapJson.contains("tilesets") && mapJson["tilesets"][0].contains("image"))
    {
        // if map contains
        //  "tilesets":[
        //         {
        //          ...
        //          "firstgid":1,
        //          "image":"tileset.png",
        //          ...
        //         }]
        tilesetPath = currentLevelInfo.tiledMapPath.parent_path() / mapJson["tilesets"][0]["image"].get<std::string>();
    }
    else
    {
        throw std::runtime_error("[ReadPathToTileset] Failed to read path to tileset");
    }

    return tilesetPath;
}

void MapLoaderSystem::RecreateBox2dWorld()
{
    auto& gameState = registry.get<GameOptions>(registry.view<GameOptions>().front());
    gameState.levelOptions.levelBox2dBounds = {};

    // Remove all entities except the GameOptions entity.
    for (auto entity : registry.view<PhysicsComponent>())
        registryWrapper.Destroy(entity);

    // Create a physics world with gravity and store it in the registry.
    gameState.physicsWorld = std::make_shared<b2World>(gameState.gravity);
    gameState.physicsWorld->SetContactListener(&contactListener);

    if (Box2dObjectRAII::GetBodyCounter() != 0)
        MY_LOG(warn, "There are still {} Box2D bodies in the memory", Box2dObjectRAII::GetBodyCounter());
    else
        MY_LOG(debug, "All Box2D bodies were destroyed");
}
