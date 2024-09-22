#include "box2d_RAII.h"
#include <cstddef>
#include <stdexcept>
#include <utility>

size_t Box2dObjectRAII::counter = 0;

Box2dObjectRAII::Box2dObjectRAII(b2Body* body, std::shared_ptr<b2World> world) : body(body), world(world)
{
    if (!body || !world)
        throw std::runtime_error("b2Body or b2World is nullptr");

    counter++;
}

Box2dObjectRAII::~Box2dObjectRAII()
{
    if (body && world)
    {
        world->DestroyBody(body);
        counter--;
    }
}

Box2dObjectRAII& Box2dObjectRAII::operator=(Box2dObjectRAII&& other) noexcept
{
    if (this != &other)
    {
        if (body && world)
        {
            world->DestroyBody(body);
        }
        body = std::exchange(other.body, nullptr);
        world = std::exchange(other.world, nullptr);

        MY_LOG(trace, "b2Body moved: {}", static_cast<void*>(body));
    }
    return *this;
}

Box2dObjectRAII::Box2dObjectRAII(Box2dObjectRAII&& other) noexcept : body(std::exchange(other.body, nullptr)), world(std::exchange(other.world, nullptr))
{}
