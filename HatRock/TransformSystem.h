#pragma once
#include <entt/entt.hpp>

class TransformSystem {
public:
    static void Update(entt::registry& registry, float deltaTime);
};