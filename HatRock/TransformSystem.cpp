#include "TransformSystem.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include <MMath.h>

void TransformSystem::Update(entt::registry& registry, float deltaTime) {

    auto view = registry.view<TransformComponent>();

    for (auto entity : view) {
        auto& transform = view.get<TransformComponent>(entity);

        //if (auto* physics = registry.try_get<PhysicsComponent>(entity)) {
        //    transform.position += physics->velocity * deltaTime;
        //    physics->velocity += physics->acceleration * deltaTime;
        //}

        transform.modelMatrix = MMath::translate(transform.position) * transform.rotationMatrix;
    }
}

