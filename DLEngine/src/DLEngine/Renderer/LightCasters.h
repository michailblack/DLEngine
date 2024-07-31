#pragma once
#include "DLEngine/Math/Vec3.h"

namespace DLEngine
{
    struct DirectionalLight
    {
        Math::Vec3 Direction;
        Math::Vec3 Luminance;
    };

    struct PointLight
    {
        Math::Vec3 Position;
        float Linear;
        Math::Vec3 Luminance;
        float Quadratic;
    };

    struct SpotLight
    {
        Math::Vec3 Position;
        float Linear;
        Math::Vec3 Direction;
        float Quadratic;
        Math::Vec3 Luminance;
        float InnerCutoffCos;
        float OuterCutoffCos;
    };
}
