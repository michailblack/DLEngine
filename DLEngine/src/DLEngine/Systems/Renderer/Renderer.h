#pragma once
#include "DLEngine/Utils/DeltaTime.h"

namespace DLEngine
{
    class Camera;

    class Renderer
    {
    public:
        static void Init();

        static void Present();

        static void OnResize(uint32_t width, uint32_t height);

        static void OnFrameBegin(DeltaTime dt);

        static void BeginScene(const Camera& camera);
        static void EndScene();
    };
}
