#pragma once
#include "DLEngine/Entity/Entity.h"
#include "DLEngine/Math/Vec2.h"

class Renderer
{
public:
    static void Draw(const Math::Sphere& entity);

    static Math::Vec2 ScreenSpaceToWorldSpace(const Math::Vec2& screenPos);

    static void OnResize(uint32_t width, uint32_t height);
};
