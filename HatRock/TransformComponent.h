#pragma once
#include <Vector.h>
#include <Matrix.h>
#include <MMath.h>

using namespace MATH;

struct TransformComponent {
    Vec3 position = Vec3(0.0f, 0.0f, 0.0f);
    Matrix4 rotationMatrix;
    Matrix4 modelMatrix;
};