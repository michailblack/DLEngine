#pragma once
#include <any>

#include "DLEngine/Renderer/Camera.h"

#include "DLEngine/Mesh/Model.h"

#include "DLEngine/Utils/DeltaTime.h"

namespace DLEngine
{
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
