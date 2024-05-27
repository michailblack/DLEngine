#include "dlpch.h"
#include "MeshSystem.h"

#include <typeindex>

namespace DLEngine
{
    bool MeshSystem::ShadingGroupKey::operator<(const ShadingGroupKey& other) const noexcept
    {
        return std::tie(MaterialType, InstanceType) < std::tie(other.MaterialType, other.InstanceType);
    }

    void MeshSystem::Init()
    {
        DL_LOG_INFO("Mesh System Initialized");
    }

    void MeshSystem::Render()
    {
        for (const auto& [key, shadingGroup] : m_ShadingGroups)
            shadingGroup->Render();
    }

    bool MeshSystem::Intersects(const Math::Ray& ray, IShadingGroup::IntersectInfo& outIntersectInfo) const
    {
        bool intersects{ false };
        for (const auto& [key, shadingGroup] : m_ShadingGroups)
            if (shadingGroup->Intersects(ray, outIntersectInfo))
                intersects = true;
        return intersects;
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
