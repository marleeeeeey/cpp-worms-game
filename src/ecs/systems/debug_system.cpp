#include "debug_system.h"
#include <ecs/components/physics_components.h>
#include <my_cpp_utils/logger.h>
#include <utils/factories/game_objects_factory.h>

DebugSystem::DebugSystem(entt::registry& registry, BaseObjectsFactory& baseObjectsFactory) : registry(registry), baseObjectsFactory(baseObjectsFactory)
{}

void DebugSystem::Update()
{
    SpawnTrailsForMarkedEntities();
}

void DebugSystem::SpawnTrailsForMarkedEntities()
{
    auto view = registry.view<PhysicsComponent, MarkForTrailDebugComponent>();
    for (auto entity : view)
    {
        auto trailDebugComponent = view.get<MarkForTrailDebugComponent>(entity);
        std::string name = MY_FMT("Trail{}", entity);
        BaseObjectsFactory::DebugSpawnOptions options;
        options.spawnPolicy = BaseObjectsFactory::SpawnPolicyBase::First;
        options.trailSize = trailDebugComponent.trailSize;
        if (trailDebugComponent.trailSize == 0)
            options.spawnPolicy = BaseObjectsFactory::SpawnPolicyBase::All;
        baseObjectsFactory.SpawnDebugVisualObject(entity, name, options);
    }
}
