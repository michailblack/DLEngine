#include "dlpch.h"
#include "MeshSystem.h"

#include <typeindex>

#include "DLEngine/Mesh/ShaderGroup.h"
#include "DLEngine/Mesh/NormalVisGroup.h"
#include "DLEngine/Mesh/HologramGroup.h"

namespace DLEngine
{
    namespace
    {
        struct MeshSystemData
        {
            std::map<std::pair<std::type_index, std::type_index>, Scope<IShaderGroup>> ShaderGroups;
        } s_Data;
    }

    void MeshSystem::Init()
    {
        s_Data.ShaderGroups.emplace(std::make_pair(
            std::make_pair(std::type_index{ typeid(NormalVisGroupMaterial) }, std::type_index{ typeid(NormalVisGroupInstance) }),
            CreateScope<NormalVisGroup>()
        ));

        s_Data.ShaderGroups.emplace(std::make_pair(
            std::make_pair(std::type_index{ typeid(HologramGroupMaterial) }, std::type_index{ typeid(HologramGroupInstance) }),
            CreateScope<HologramGroup>()
        ));
    }

    void MeshSystem::Render()
    {
        for (const auto& shaderGroup : s_Data.ShaderGroups | std::views::values)
            shaderGroup->Render();
    }

    void MeshSystem::AddModel(const Ref<Model>& model, const std::any& material, const std::any& instance)
    {
        std::type_index materialType = std::type_index{ material.type() };
        std::type_index instanceType = std::type_index{ instance.type() };

        auto it{ s_Data.ShaderGroups.find(std::make_pair(materialType, instanceType)) };

        DL_ASSERT(it != s_Data.ShaderGroups.end(), "No shader group found for material and instance types");

        it->second->AddModel(model, material, instance);
    }

    bool MeshSystem::Intersects(const Math::Ray& ray, const Math::Vec3& cameraForward, Ref<IDragger>& outMeshDragger)
    {
        bool intersects{ false };
        IShaderGroup::IntersectInfo intersectInfo;
        for (const auto& shaderGroup : s_Data.ShaderGroups | std::views::values)
        {
            if (shaderGroup->Intersects(ray, intersectInfo))
            {
                outMeshDragger = shaderGroup->CreateMeshDragger(ray, cameraForward, intersectInfo);
                intersects = true;
            }
        }
        return intersects;
    }
}
