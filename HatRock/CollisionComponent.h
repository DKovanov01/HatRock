#pragma once
#include <Vector.h>

using namespace MATH;

struct CollisionComponent {
    Vec3 bounds = Vec3(1.0f, 1.0f, 1.0f);

    bool isColliding = false;
    bool isTrigger = false;
};