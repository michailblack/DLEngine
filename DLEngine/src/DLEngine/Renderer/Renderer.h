#pragma once
#include "Scene.h"
#include "DLEngine/Core/Window.h"

class Renderer
{
public:
    static void Clear(Math::Vec<3, uint8_t> color);
    static void RenderScene(const Scene& scene);
    static void SwapFramebuffer();
};
