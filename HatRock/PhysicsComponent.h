#pragma once

#include <Vector.h>

using namespace MATH;

struct PhysicsComponent {
    Vec3 velocity = Vec3(0.0f, 0.0f, 0.0f);
    Vec3 acceleration = Vec3(0.0f, 0.0f, 0.0f);
    float mass = 1.0f;
};