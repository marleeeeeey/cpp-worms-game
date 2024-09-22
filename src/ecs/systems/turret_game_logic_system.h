#pragma once
#include <entt/entt.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/factories/game_objects_factory.h>

class TurretGameLogicSystem
{
    entt::registry& registry;
    GameObjectsFactory& gameObjectsFactory;
    CoordinatesTransformer& coordinatesTransformer;
public:
    TurretGameLogicSystem(entt::registry& registry, GameObjectsFactory& gameObjectsFactory, CoordinatesTransformer& coordinatesTransformer);
    void Update();
};