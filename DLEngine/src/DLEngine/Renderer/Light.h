#pragma once
#include "DLEngine/Math/Vec3.h"

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

struct Environment
{
    Math::Vec3 IndirectLightingColor;
    DirectionalLight Sun;
    std::vector<PointLight> PointLights;
    float Exposure;
};
