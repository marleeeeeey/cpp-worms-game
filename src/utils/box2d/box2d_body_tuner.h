#pragma once
#include "utils/game_options.h"
#include <box2d/box2d.h>
#include <ecs/components/physics_components.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/box2d/box2d_RAII.h>
#include <utils/box2d/box2d_body_options.h>
#include <utils/coordinates_transformer.h>

class Box2dBodyTuner
{
    entt::registry& registry;
    CoordinatesTransformer coordinatesTransformer;
    GameOptions& gameState;
public: //////////////////////////////////////////// Constructor. ///////////////////////////////////////////
    Box2dBodyTuner(entt::registry& registry);
public: ////////////////////////////////////// Create physics component. ////////////////////////////////////
    PhysicsComponent& CreatePhysicsComponent(entt::entity entity, const glm::vec2& posWorld, float angle, const Box2dBodyOptions& options);
public: //////////////// Get physics component or throw exception. May be used to get options. //////////////
    PhysicsComponent& GetPhysicsComponent(entt::entity entity);
public: /////////////////////////////////////////// Options setters. /////////////////////////////////////////
    void ApplyOption(entt::entity entity, const Box2dBodyOptions::Fixture& fixture);
    void ApplyOption(entt::entity entity, const Box2dBodyOptions::Shape& option);
    void ApplyOption(entt::entity entity, const Box2dBodyOptions::Sensor& option);
    void ApplyOption(entt::entity entity, const Box2dBodyOptions::MovementPolicy& option);
    void ApplyOption(entt::entity entity, const Box2dBodyOptions::AnglePolicy& option);
    void ApplyOption(entt::entity entity, const Box2dBodyOptions::CollisionPolicy& option);
    void ApplyOption(entt::entity entity, const Box2dBodyOptions::BulletPolicy& option);
    void ApplyOption(entt::entity entity, const Box2dBodyOptions::Hitbox& hitbox);
private: ///////////////////////////////////// Create empty physics body. ///////////////////////////////////
    b2Body* CreatePhysicsBodyWithNoShape(entt::entity entity, const glm::vec2& posWorld);
private: ////////////////////////////////// Add simple fixtures to the body. ////////////////////////////////
    void AddBoxFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld);
    void AddCircleFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld);
    void AddVerticalCapsuleFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld);
    void AddThinSensorBelowTheBody(b2Body* body, const glm::vec2& sizeWorld);
private: /////////////////////////////////// Remove fixtures from the body. /////////////////////////////////
    void RemoveAllFixturesExceptSensorsFromTheBody(b2Body* body);
    void RemoveAllSensorsFromTheBody(b2Body* body);
private: ////////////////////////////////////////// Get fixture def. ////////////////////////////////////////
    b2FixtureDef CalcFixtureDefFromOptions(const Box2dBodyOptions::Fixture& options);
};
