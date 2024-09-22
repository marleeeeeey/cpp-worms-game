#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <utils/factories/box2d_body_creator.h>
#include <utils/game_options.h>
#include <vector>

class Box2dUtils
{
    entt::registry& registry;
    GameOptions& gameState;
    Box2dBodyCreator box2dBodyCreator;
    CoordinatesTransformer coordinatesTransformer;
public:
    Box2dUtils(entt::registry& registry);
public: ////////////////////////////////////////// Client methods. ////////////////////////////////////////
    void ApplyForceToPhysicalBodies(std::vector<entt::entity> physicalEntities, const glm::vec2& forceCenterWorld, float force);
};