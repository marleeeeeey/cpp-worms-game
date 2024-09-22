#pragma once
#include "utils/box2d/box2d_math.h"
#include "utils/sdl/sdl_colors.h"
#include <box2d/b2_math.h>
#include <box2d/box2d.h>
#include <ecs/components/physics_components.h>
#include <entt/entt.hpp>
#include <utils/box2d/box2d_glm_operators.h>
#include <utils/sdl/sdl_primitives_renderer.h>

namespace details
{

enum class DrawBoudingBoxesOptions
{
    DrawSensors = 1 << 0,
    DrawEverythingExceptSensors = 1 << 1,
    _entt_enum_as_bitmask
};

template <typename EnttViewT>
void DrawBoudingBoxesAdvanced(
    SdlPrimitivesRenderer& pr, CoordinatesTransformer& ct, EnttViewT view, DrawBoudingBoxesOptions options = DrawBoudingBoxesOptions::DrawEverythingExceptSensors,
    std::optional<ColorName> colorOpt = std::nullopt)
{
    for (auto entity : view)
    {
        auto color = GetRandomColorName();
        if (colorOpt.has_value())
            color = colorOpt.value();

        const auto& physicsInfo = view.template get<PhysicsComponent>(entity);

        auto body = physicsInfo.bodyRAII->GetBody();
        for (auto fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
        {
            if (!(options & DrawBoudingBoxesOptions::DrawSensors))
            {
                if (fixture->IsSensor())
                    continue;
            }

            if (!(options & DrawBoudingBoxesOptions::DrawEverythingExceptSensors))
            {
                if (!fixture->IsSensor())
                    continue;
            }

            const auto shape = fixture->GetShape();
            if (shape->GetType() == b2Shape::e_circle)
            {
                const auto circleShape = static_cast<b2CircleShape*>(shape);
                const glm::vec2 centerWorld = ct.PhysicsToWorld(body->GetWorldPoint(circleShape->m_p));
                float radiusWorld = ct.PhysicsToWorld(circleShape->m_radius);
                pr.RenderCircle(centerWorld, radiusWorld, color);
            }
            else if (shape->GetType() == b2Shape::e_polygon)
            {
                const auto polygonShape = static_cast<b2PolygonShape*>(shape);

                auto centerPhysics = body->GetWorldPoint(polygonShape->m_centroid);
                auto rectPhysics = TranformBox2dPolygonShapeToRectagleSize(polygonShape);

                auto rectangleCenterPixel = ct.PhysicsToWorld(centerPhysics);
                auto rectangleSizePixel = ct.PhysicsToWorld(rectPhysics);

                pr.RenderRect(rectangleCenterPixel, rectangleSizePixel, body->GetAngle(), color);
            }
        }
    }
}

} // namespace details

template <typename EnttViewT>
void DrawBoudingBoxes(SdlPrimitivesRenderer& pr, CoordinatesTransformer& ct, EnttViewT view, std::optional<ColorName> colorOpt = std::nullopt)
{
    details::DrawBoudingBoxesAdvanced(pr, ct, view, details::DrawBoudingBoxesOptions::DrawEverythingExceptSensors, colorOpt);
}

template <typename EnttViewT>
void DrawSensorBoxes(SdlPrimitivesRenderer& pr, CoordinatesTransformer& ct, EnttViewT view, std::optional<ColorName> colorOpt = std::nullopt)
{
    details::DrawBoudingBoxesAdvanced(pr, ct, view, details::DrawBoudingBoxesOptions::DrawSensors, colorOpt);
}