#pragma once
#include "DLEngine/Math/Mat4x4.h"

#include "DLEngine/Renderer/Material.h"

struct SphereInstance
{
    Math::Sphere Sphere;
    Material Mat;
};

struct PlaneInstance
{
    Math::Plane Plane;
    Material Mat;
};

struct MeshInstance
{
    Math::Mat4x4 Transform;
    Math::Mat4x4 InvTransform;
    Material Mat;
};
