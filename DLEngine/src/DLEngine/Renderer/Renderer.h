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

        static void OnFrameBegin(DeltaTime dt);

        static void BeginScene(const Camera& camera);
        static void EndScene();

        static void Submit(const Ref<Model>& model, const std::any& material, const std::any& instance);
    };
}
