#pragma once
#include "DLEngine/Core/Base.h"

#include "DLEngine/Systems/Mesh/ShadingGroup.h"
#include "DLEngine/Systems/Mesh/ShadingGroupStructs.h"

#include <map>
#include <typeindex>

namespace DLEngine
{
    class Model;

    class MeshSystem
    {
    public:
        static MeshSystem& Get()
        { 
            static MeshSystem s_Instance;
            return s_Instance;
        }

        void Init();
        void Render();

        bool Intersects(const Math::Ray& ray, IShadingGroup::IntersectInfo& outIntersectInfo) const noexcept;

        template <typename TMaterial, typename TInstance>
        void CreateShadingGroup(const ShadingGroupDesc& desc);

        template <typename TMaterial, typename TInstance>
        void Add(const Ref<Model>& model, std::vector<TMaterial> meshMaterials, const TInstance& instance);

        void ToggleNormalVis() noexcept;

    private:
        MeshSystem() = default;

    private:
        void InitNormalVisGroup();
        void InitHologramGroup();
        void InitTextureOnlyGroup();
        void InitEmissionGroup();
        void InitLitGroup();

    private:
        struct ShadingGroupKey
        {
            std::type_index MaterialType;
            std::type_index InstanceType;

            bool operator<(const ShadingGroupKey& other) const noexcept;  
        };
        friend std::hash<ShadingGroupKey>;

    private:
        std::map<ShadingGroupKey, Scope<IShadingGroup>> m_ShadingGroups;
    };

    template <typename TMaterial, typename TInstance>
    void MeshSystem::CreateShadingGroup(const ShadingGroupDesc& desc)
    {
        ShadingGroupKey key{
            .MaterialType = std::type_index{ typeid(TMaterial) },
            .InstanceType = std::type_index{ typeid(TInstance) }
        };

        DL_ASSERT(m_ShadingGroups.find(key) == m_ShadingGroups.end(), "Shading group already exists");

        m_ShadingGroups[key] = CreateScope<ShadingGroup<TMaterial, TInstance>>(desc);

        DL_LOG_INFO("Created shading group: {}", desc.Name);
    }

    template <typename TMaterial, typename TInstance>
    void MeshSystem::Add(const Ref<Model>& model, std::vector<TMaterial> meshMaterials, const TInstance& instance)
    {
        ShadingGroupKey key{
            .MaterialType = std::type_index{ typeid(TMaterial) },
            .InstanceType = std::type_index{ typeid(TInstance) }
        };

        auto shadingGroup{ m_ShadingGroups.find(key) };

        DL_ASSERT(shadingGroup != m_ShadingGroups.end(), "Shading group does not exist");

        static_cast<ShadingGroup<TMaterial, TInstance>&>(*shadingGroup->second).AddModel(model, meshMaterials, instance);

        ShadingGroupKey normalVisKey{
            .MaterialType = std::type_index{ typeid(ShadingGroupStruct::Material::Null) },
            .InstanceType = std::type_index{ typeid(ShadingGroupStruct::Instance::Null) }
        };

        auto normalVisShadingGroup{ m_ShadingGroups.find(normalVisKey) };

        DL_ASSERT(normalVisShadingGroup != m_ShadingGroups.end(), "NormalVis shading group does not exist");

        std::vector<ShadingGroupStruct::Material::Null> nullMaterials{};
        nullMaterials.resize(model->GetMeshesCount());

        ShadingGroupStruct::Instance::Null nullInstance{};
        nullInstance.TransformID = instance.TransformID;
        
        static_cast<ShadingGroup<ShadingGroupStruct::Material::Null, ShadingGroupStruct::Instance::Null>&>(*normalVisShadingGroup->second).AddModel(model, nullMaterials, nullInstance);
    }
}
