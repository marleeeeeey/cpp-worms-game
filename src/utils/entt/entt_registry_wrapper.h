#pragma once
#include <entt/entt.hpp>

class EnttRegistryWrapper
{
    entt::registry& registry;
    std::map<entt::entity, std::string> entityNamesById;
    std::map<entt::entity, std::string> removedEntityNamesById;
public:
    explicit EnttRegistryWrapper(entt::registry& registry);
    EnttRegistryWrapper(const EnttRegistryWrapper&) = delete;
    EnttRegistryWrapper& operator=(const EnttRegistryWrapper&) = delete;
public: /////////////// Methods for debug - use in client code. /////////////
    entt::entity Create(const std::string& name);
    void Destroy(entt::entity entity);
    void LogAllEntitiesByTheirNames();
    std::string TryGetName(entt::entity entity);
    // Get original registry.
    operator entt::registry&() { return registry; }
};