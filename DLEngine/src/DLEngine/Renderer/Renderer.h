#pragma once
#include "DLEngine/Entity/Entity.h"

class Renderer
{
public:
    static void Draw(const Math::Sphere& entity);

    static Math::Vec2f ScreenSpaceToWorldSpace(const Math::Vec2<int32_t>& screenPos);

    static void OnResize(uint32_t width, uint32_t height);
};
