#pragma once
#include "DLEngine/Math/Vec3.h"

namespace DLEngine
{
    struct DirectionalLight
    {
        Math::Vec3 Direction;
        float SolidAngle;
        Math::Vec3 Luminance;
    };

    struct PointLight
    {
        Math::Vec3 Position;
        float Radius;
        Math::Vec3 Luminance;
    };

    struct SpotLight
    {
        Math::Vec3 Position;
        float InnerCutoffCos;
        Math::Vec3 Direction;
        float OuterCutoffCos;
        Math::Vec3 Luminance;
        float Radius;
    };
}
