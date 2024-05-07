#pragma once
#include "DLEngine/Math/Vec3.h"

namespace DLEngine
{
    struct DirectionalLight
    {
        Math::Vec3 Direction;
        Math::Vec3 Color;
    };

    struct PointLight
    {
        Math::Vec3 Position;
        Math::Vec3 Color;
        float Linear;
        float Quadratic;
    };

    struct SpotLight
    {
        Math::Vec3 Position;
        Math::Vec3 Direction;
        Math::Vec3 Color;
        float Linear;
        float Quadratic;
        float InnerCutoffCos;
        float OuterCutoffCos;
    };

    struct Environment
    {
        Math::Vec3 IndirectLightingColor;
        DirectionalLight Sun;
        std::vector<PointLight> PointLights;
        std::vector<SpotLight> SpotLights;
        float Exposure;
    };
}
