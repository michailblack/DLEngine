#include "dlpch.h"
#include "MeshSystem.h"

#include <typeindex>

namespace DLEngine
{
    /*namespace
    {
        struct ShaderGroupKey
        {
            std::type_index MaterialType;
            std::type_index InstanceType;

            bool operator<(const ShaderGroupKey& other) const noexcept
            {
                return std::tie(MaterialType, InstanceType) < std::tie(other.MaterialType, other.InstanceType);
            }
        };

        struct MeshSystemData
        {
            struct
            {
                Scope<NormalVisGroup> NormalVis;
                std::map<ShaderGroupKey, Scope<IShadingGroup>> ShaderGroups;
            } ShadingGroups;
        } s_Data;
    }

    void MeshSystem::Init()
    {
        s_Data.ShadingGroups.NormalVis = CreateScope<NormalVisGroup>();
    }

    void MeshSystem::Render()
    {
        s_Data.ShadingGroups.NormalVis->Render();
    }

    void MeshSystem::Add(const Ref<Model>& model, const NormalVisGroup::Instance& instance)
    {
        s_Data.ShadingGroups.NormalVis->AddModel(model, instance);
    }*/

    bool MeshSystem::ShadingGroupKey::operator<(const ShadingGroupKey& other) const noexcept
    {
        return std::tie(MaterialType, InstanceType) < std::tie(other.MaterialType, other.InstanceType);
    }

    void MeshSystem::Init()
    {

    }

    void MeshSystem::Render()
    {
        for (const auto& [key, shadingGroup] : m_ShadingGroups)
            shadingGroup->Render();
    }
}

namespace std
{
    template <>
    struct hash<DLEngine::MeshSystem::ShadingGroupKey>
    {
        size_t operator()(const DLEngine::MeshSystem::ShadingGroupKey& key) const noexcept
        {
            return std::hash<std::type_index>{}(key.MaterialType) ^ std::hash<std::type_index>{}(key.InstanceType);
        }
    };
}
