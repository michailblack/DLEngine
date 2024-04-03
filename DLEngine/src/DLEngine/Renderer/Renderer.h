#pragma once
#include "DLEngine/Core/Window.h"
#include "DLEngine/Entity/Entity.h"

class Renderer
{
public:
    static void Clear(Math::Vec<3, uint8_t> color);
    static void Submit(const Math::Sphere& entity);
    static void SwapFramebuffer();
};
