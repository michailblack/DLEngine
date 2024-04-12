#pragma once
#include "Camera.h"
#include "DLEngine/Entity/Entity.h"
#include "DLEngine/Math/Vec2.h"

class Renderer
{
public:
    static void BeginScene(const Camera& camera);
    static void EndScene();

    static void Submit(const Math::Sphere& sphere);

    static void OnResize(uint32_t width, uint32_t height);
};
