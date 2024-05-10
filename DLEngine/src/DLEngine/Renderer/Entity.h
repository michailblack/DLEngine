#pragma once
#include "DLEngine/Math/Mat4x4.h"

#include "DLEngine/Renderer/Material.h"

namespace DLEngine
{
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
}
