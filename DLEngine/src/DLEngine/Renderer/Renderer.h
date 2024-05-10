#pragma once
#include "DLEngine/Renderer/Camera.h"
#include "DLEngine/Renderer/NormalVisGroup.h"
#include "DLEngine/Renderer/HologramGroup.h"

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

        static void SubmitToNormalVisGroup(const Ref<Model>& model, const std::vector<NormalVisGroup::Instance>& instances);
        static void SubmitToHologramGroup(const Ref<Model>& model, const std::vector<HologramGroup::Instance>& instances);

        static Math::Ray GetRay(uint32_t mouseX, uint32_t mouseY);
    };
}
