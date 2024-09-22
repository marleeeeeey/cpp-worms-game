#pragma once
#include <ecs/components/animation_components.h>
#include <utils/resources/resource_manager.h>

class ComponentsFactory
{
    ResourceManager& resourceManager;
public:
    ComponentsFactory(ResourceManager& resourceManager);

    AnimationComponent CreateAnimationComponent(const std::string& animationName, const std::string& tagName, ResourceManager::TagProps tagProps);
};