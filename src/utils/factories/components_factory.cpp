#include "components_factory.h"

ComponentsFactory::ComponentsFactory(ResourceManager& resourceManager) : resourceManager(resourceManager)
{}

AnimationComponent ComponentsFactory::CreateAnimationComponent(const std::string& animationName, const std::string& tagName, ResourceManager::TagProps tagProps)
{
    AnimationComponent animationInfo;
    animationInfo.animation = resourceManager.GetAnimation(animationName, tagName, tagProps);
    animationInfo.isPlaying = true;
    return animationInfo;
}
