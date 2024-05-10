#include "dlpch.h"
#include "MeshSystem.h"

namespace DLEngine
{
    namespace
    {
        struct MeshSystemData
        {
            Scope<NormalVisGroup> NormalVisGroup;
            Scope<HologramGroup> HologramGroup;
        } s_Data;
    }

    void MeshSystem::Init()
    {
        s_Data.NormalVisGroup = CreateScope<NormalVisGroup>();
        s_Data.HologramGroup = CreateScope<HologramGroup>();
    }

    void MeshSystem::Render()
    {
        s_Data.NormalVisGroup->Render();
        s_Data.HologramGroup->Render();
    }

    void MeshSystem::AddToNormalVisGroup(const Ref<Model>& model, const std::vector<NormalVisGroup::Instance>& instances)
    {
        s_Data.NormalVisGroup->AddModel(model, instances);
    }

    void MeshSystem::AddToHologramGroup(const Ref<Model>& model, const std::vector<HologramGroup::Instance>& instances)
    {
        s_Data.HologramGroup->AddModel(model, instances);
    }
}