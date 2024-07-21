#pragma once
#include "DLEngine/Math/Vec3.h"

namespace DLEngine
{
    struct DirectionalLight
    {
        Math::Vec3 Direction;
        Math::Vec3 Radiance;
        float SolidAngle;
    };

    struct PointLight
    {
        Math::Vec3 Position;
        Math::Vec3 Radiance;
        float Radius;
    };

    struct SpotLight
    {
        Math::Vec3 Position;
        Math::Vec3 Direction;
        Math::Vec3 Radiance;
        float Radius;
        float InnerCutoffCos;
        float OuterCutoffCos;
    };
}
