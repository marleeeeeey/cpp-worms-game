#include "box2d_entt_contact_listener.h"
#include <utils/logger.h>

Box2dEnttContactListener::Box2dEnttContactListener(EnttRegistryWrapper& registryWrapper) : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry())
{}

void Box2dEnttContactListener::BeginContact(b2Contact* contact)
{
    if (auto validEntities = GetValidEntities(contact))
    {
        auto [entityWithPropsA, entityWithPropsB] = *validEntities;

        for (auto& listener : contactListenersByType[ContactType::Begin])
        {
            if (!entityWithPropsA.isSensor && !entityWithPropsB.isSensor)
            {
                listener({entityWithPropsA.entity, entityWithPropsB.entity, contact});
            }
        }

        for (auto& listener : contactListenersByType[ContactType::BeginSensor])
        {
            if (entityWithPropsA.isSensor || entityWithPropsB.isSensor)
            {
                listener({entityWithPropsA.entity, entityWithPropsB.entity, contact});
            }
        }
    }
}

void Box2dEnttContactListener::EndContact(b2Contact* contact)
{
    if (auto validEntities = GetValidEntities(contact))
    {
        auto [entityWithPropsA, entityWithPropsB] = *validEntities;

        for (auto& listener : contactListenersByType[ContactType::End])
        {
            if (!entityWithPropsA.isSensor && !entityWithPropsB.isSensor)
            {
                listener({entityWithPropsA.entity, entityWithPropsB.entity, contact});
            }
        }

        for (auto& listener : contactListenersByType[ContactType::EndSensor])
        {
            if (entityWithPropsA.isSensor || entityWithPropsB.isSensor)
            {
                listener({entityWithPropsA.entity, entityWithPropsB.entity, contact});
            }
        }
    }
}

std::optional<std::pair<Box2dEnttContactListener::EntityWithProperties, Box2dEnttContactListener::EntityWithProperties>> Box2dEnttContactListener::GetValidEntities(
    b2Contact* contact)
{
    b2Body* bodyA = contact->GetFixtureA()->GetBody();
    b2Body* bodyB = contact->GetFixtureB()->GetBody();

    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    auto pointerA = bodyA->GetUserData().pointer;
    auto pointerB = bodyB->GetUserData().pointer;
    if (pointerA == 0 || pointerB == 0)
    {
        MY_LOG(warn, "One of the bodies has no user data. pointerA: {}, pointerB: {}", pointerA, pointerB);
        return std::nullopt;
    }

    auto entityA = static_cast<entt::entity>(pointerA);
    auto entityB = static_cast<entt::entity>(pointerB);

    if (!registry.valid(entityA))
    {
        MY_LOG(debug, "EntityA is not valid. entityA: {}, name: {}", entityA, registryWrapper.TryGetName(entityA));
    }

    if (!registry.valid(entityB))
    {
        MY_LOG(debug, "EntityB is not valid. entityB: {}, name: {}", entityB, registryWrapper.TryGetName(entityB));
    }

    if (registry.valid(entityA) && registry.valid(entityB))
    {
        EntityWithProperties entityWithPropertiesA{fixtureA->IsSensor(), entityA};
        EntityWithProperties entityWithPropertiesB{fixtureB->IsSensor(), entityB};
        return std::make_pair(entityWithPropertiesA, entityWithPropertiesB);
    }

    return std::nullopt;
}

void Box2dEnttContactListener::SubscribeContact(ContactType contactType, ContactListener listener)
{
    contactListenersByType[contactType].push_back(listener);
}
