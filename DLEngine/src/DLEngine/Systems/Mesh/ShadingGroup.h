#pragma once
#include "DLEngine/Core/Base.h"

#include "DLEngine/DirectX/BufferLayout.h"
#include "DLEngine/DirectX/ConstantBuffer.h"
#include "DLEngine/DirectX/PipelineState.h"
#include "DLEngine/DirectX/VertexBuffer.h"

#include "DLEngine/Systems/Mesh/Mesh.h"

#include <concepts>

namespace DLEngine
{
    class Model;

    class IShadingGroup
    {
    public:
        struct IntersectInfo
        {
            Mesh::IntersectInfo MeshIntersectInfo;
            Ref<Model> Model;
            uint32_t TransformID{ 0u };
        };

    public:
        IShadingGroup() = default;

        IShadingGroup(const IShadingGroup& other) = delete;
        IShadingGroup(IShadingGroup&& other) noexcept = delete;
        IShadingGroup& operator=(const IShadingGroup& other) = delete;
        IShadingGroup& operator=(IShadingGroup&& other) noexcept = delete;

        virtual ~IShadingGroup() = default;

        virtual void Render() = 0;
        virtual bool Intersects(const Math::Ray& ray, IShadingGroup::IntersectInfo& outIntersectInfo) const noexcept = 0;

        void ToggleRender() noexcept { m_Render = !m_Render; }

    protected:
        bool m_Render{ true };
    };

    struct ShadingGroupDesc
    {
        std::string Name;

        // PipelineStateDesc::Layout is ignored as it is created in the ShadingGroup constructor
        PipelineState PipelineState;

        BufferLayout InstanceBufferLayout;

        bool Render{ true };
    };

    template <typename T>
    concept EqualityComparable = requires(const T& a, const T& b)
    {
        { a == b } -> std::same_as<bool>;
    };

    template <typename TMaterial>
    concept Settable = requires(const TMaterial& material)
    {
        { material.Set() } -> std::same_as<void>;
    };

    template <typename TMaterial>
    concept MaterialConcept = EqualityComparable<TMaterial> && Settable<TMaterial>;

    template <typename TInstance>
    concept Transformable = std::same_as<decltype(std::declval<TInstance>().TransformID), uint32_t>;

    template <typename TInstance>
    concept ConvertableToGPU = requires(const TInstance& instance)
    {
        { instance.ConvertToGPU() } -> std::same_as<typename TInstance::GPUType>;
    };

    template <typename TInstance>
    concept InstanceConcept = Transformable<TInstance> && ConvertableToGPU<TInstance>;

    template <MaterialConcept TMaterial, InstanceConcept TInstance>
    class ShadingGroup
        : public IShadingGroup
    {
    public:
        ShadingGroup(const ShadingGroupDesc& desc);

        void AddModel(const Ref<Model>& model, std::vector<TMaterial> meshMaterials, const TInstance& instance) noexcept;
        void Render() override;
        bool Intersects(const Math::Ray& ray, IShadingGroup::IntersectInfo& outIntersectInfo) const noexcept override;

    private:
        void UpdateInstanceBuffer() noexcept;

    private:
        struct PerMaterial
        {
            TMaterial Material{};
            std::vector<TInstance> Instances;
        };

        struct PerMesh
        {
            std::vector<PerMaterial> Materials;
        };

        struct PerModel
        {
            Ref<Model> Model;
            std::vector<PerMesh> Meshes;
        };

    private:
        struct MeshInstance
        {
            Math::Mat4x4 MeshToModel{};
            Math::Mat4x4 ModelToMesh{};
        };

    private:
        std::vector<PerModel> m_Models;

        PipelineState m_PipelineState{};
        VertexBuffer m_InstanceBuffer{};
        ConstantBuffer m_MeshInstanceCB{};
    };
}

#include "ShadingGroup.inl"
