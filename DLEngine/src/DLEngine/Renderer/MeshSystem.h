#pragma once
#include "DLEngine/Renderer/NormalVisGroup.h"
#include "DLEngine/Renderer/HologramGroup.h"

namespace DLEngine
{
    class MeshSystem
    {
    public:
        static void Init();

        static void Render();

        static void AddToNormalVisGroup(const Ref<Model>& model, const std::vector<NormalVisGroup::Instance>& instances);
        static void AddToHologramGroup(const Ref<Model>& model, const std::vector<HologramGroup::Instance>& instances);
    };
}
