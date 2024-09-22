#pragma once
#include "entt/entity/fwd.hpp"
#include "utils/box2d/box2d_body_tuner.h"
#include <ecs/components/physics_components.h>
#include <utils/box2d/box2d_RAII.h>
#include <utils/box2d/box2d_body_options.h>
#include <utils/coordinates_transformer.h>
#include <utils/game_options.h>

class Box2dBodyCreator
{
    Box2dBodyTuner bodyTuner;
public:
    Box2dBodyCreator(entt::registry& registry);
    // entity is used to store entity in the physics body.
    // Size of object getting from options.hitbox.sizeWorld.
    PhysicsComponent& CreatePhysicsBody(entt::entity entity, const glm::vec2& posWorld, float angle, const Box2dBodyOptions& options);
    // Overwrite size of object in options.hitbox.sizeWorld with sizeWorld.
    PhysicsComponent& CreatePhysicsBody(entt::entity entity, const glm::vec2& posWorld, const glm::vec2& sizeWorld, float angle, Box2dBodyOptions options);
};