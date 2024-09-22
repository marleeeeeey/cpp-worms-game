#include "coordinates_transformer.h"
#include <my_cpp_utils/config.h>

CoordinatesTransformer::CoordinatesTransformer(entt::registry& registry)
  : gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), box2DtoWorld(utils::GetConfig<float, "CoordinatesTransformer.box2DtoWorld">()),
    worldToBox2D(1.0f / box2DtoWorld)
{}

glm::vec2 CoordinatesTransformer::WorldToScreen(const glm::vec2& posWorld, Type type) const
{
    auto& windowOpt = gameState.windowOptions;

    if (type == Type::Length)
        return posWorld * windowOpt.cameraScale;

    glm::vec2 transformedPosition = (posWorld - windowOpt.cameraCenterSdl) * windowOpt.cameraScale + windowOpt.windowSize / 2.0f;
    return transformedPosition;
}

float CoordinatesTransformer::WorldToScreen(float valueWorld) const
{
    return valueWorld * gameState.windowOptions.cameraScale;
}

glm::vec2 CoordinatesTransformer::ScreenToWorld(const glm::vec2& posScreen, Type type) const
{
    auto& windowOpt = gameState.windowOptions;

    if (type == Type::Length)
        return posScreen / windowOpt.cameraScale;

    glm::vec2 worldPosition = ((posScreen - windowOpt.windowSize / 2.0f) / windowOpt.cameraScale) + windowOpt.cameraCenterSdl;
    return worldPosition;
}

float CoordinatesTransformer::ScreenToWorld(float valueScreen) const
{
    return valueScreen / gameState.windowOptions.cameraScale;
}

b2Vec2 CoordinatesTransformer::WorldToPhysics(const glm::vec2& posWorld, [[maybe_unused]] Type type) const
{
    return b2Vec2(posWorld.x * worldToBox2D, posWorld.y * worldToBox2D);
}

float CoordinatesTransformer::WorldToPhysics(float worldValue) const
{
    return worldValue * worldToBox2D;
}

glm::vec2 CoordinatesTransformer::PhysicsToWorld(const b2Vec2& posPhysics, [[maybe_unused]] Type type) const
{
    return glm::vec2(posPhysics.x * box2DtoWorld, posPhysics.y * box2DtoWorld);
}

float CoordinatesTransformer::PhysicsToWorld(float physicsValue) const
{
    return physicsValue * box2DtoWorld;
}

b2Vec2 CoordinatesTransformer::ScreenToPhysics(const glm::vec2& posSreen, Type type) const
{
    return WorldToPhysics(ScreenToWorld(posSreen, type), type);
}

float CoordinatesTransformer::ScreenToPhysics(float valueScreen) const
{
    return WorldToPhysics(ScreenToWorld(valueScreen));
}

glm::vec2 CoordinatesTransformer::PhysicsToScreen(const b2Vec2& posPhysics, Type type) const
{
    return WorldToScreen(PhysicsToWorld(posPhysics, type), type);
}

float CoordinatesTransformer::PhysicsToScreen(float valuePhysics) const
{
    return WorldToScreen(PhysicsToWorld(valuePhysics));
}