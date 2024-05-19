#pragma once
#include "DLEngine/Core/Base.h"

#include "DLEngine/DirectX/ConstantBuffers.h"
#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/InputLayout.h"
#include "DLEngine/DirectX/Shaders.h"

#include "DLEngine/Mesh/IDragger.h"
#include "DLEngine/Mesh/Model.h"

#include <any>
#include <ranges>

namespace DLEngine
{
    class IShaderGroup
    {
    public:
        struct IntersectInfo
        {
            Model::IntersectInfo ModelIntersectInfo;
            uint32_t ModelIndex{ 0u };
            uint32_t MeshIndex{ 0u };
            uint32_t InstanceIndex{ 0u };
        };

    public:
        virtual ~IShaderGroup() = default;

        virtual void AddModel(const Ref<Model>& model, const std::any& material, const std::any& instance) = 0;
        virtual void Render() = 0;

        virtual bool Intersects(const Math::Ray& ray, IShaderGroup::IntersectInfo& outIntersectInfo) const = 0;
        virtual Ref<MeshDragger> CreateMeshDragger(const Math::Ray& ray, const Math::Vec3& cameraForward, const IShaderGroup::IntersectInfo& intersectInfo) = 0;
    };

    template <typename Material, typename Instance>
    class ShaderGroup
        : public IShaderGroup
    {
    public:
        struct PerMesh
        {
            std::any Material;
            std::vector<std::any> Instances;
        };

        struct PerModel
        {
            Ref<Model> Model;
            std::vector<PerMesh> PerMesh;
        };

    public:
        ShaderGroup() = default;
        ~ShaderGroup() override = default;

        ShaderGroup(const ShaderGroup&) = delete;
        ShaderGroup(ShaderGroup&&) = delete;
        ShaderGroup& operator=(const ShaderGroup&) = delete;
        ShaderGroup& operator=(ShaderGroup&&) = delete;

        void AddModel(const Ref<Model>& model, const std::any& material, const std::any& instance) final;
        void Render() final;

        bool Intersects(const Math::Ray& ray, IShaderGroup::IntersectInfo& outIntersectInfo) const final;

    private:
        void UpdateInstanceBuffer() const;

    protected:
        virtual void UpdateAndSetPerDrawBuffer(uint32_t modelIndex, uint32_t meshIndex, uint32_t instanceIndex) const = 0;
        virtual bool IntersectsInstance(const Math::Ray& ray, IShaderGroup::IntersectInfo& outIntersectInfo) const = 0;

    protected:
        std::vector<PerModel> m_Models;
        Ref<PerInstanceBuffer<Instance>> m_InstanceBuffer;

        Ref<InputLayout> m_InputLayout;

        Ref<VertexShader> m_VertexShader;
        Ref<PixelShader> m_PixelShader;

        Microsoft::WRL::ComPtr<ID3D11RasterizerState2> m_RasterizerState;
    };

    template <typename Material, typename Instance>
    bool ShaderGroup<Material, Instance>::Intersects(const Math::Ray& ray, IShaderGroup::IntersectInfo& outIntersectInfo) const
    {
        bool intersects{ false };
        for (uint32_t modelIndex{ 0u }; modelIndex < m_Models.size(); ++modelIndex)
        {
            const auto& perModel{ m_Models[modelIndex] };
            for (uint32_t meshIndex{ 0u }; meshIndex < perModel.PerMesh.size(); ++meshIndex)
            {
                const auto& perMesh{ perModel.PerMesh[meshIndex] };
                for (uint32_t instanceIndex{ 0u }; instanceIndex < perMesh.Instances.size(); ++instanceIndex)
                {
                    IShaderGroup::IntersectInfo intersectInfo{ outIntersectInfo };
                    intersectInfo.ModelIndex = modelIndex;
                    intersectInfo.MeshIndex = meshIndex;
                    intersectInfo.InstanceIndex = instanceIndex;
                    if (IntersectsInstance(ray, intersectInfo))
                    {
                        outIntersectInfo = intersectInfo;
                        intersects = true;
                    }
                }
            }
        }
        return intersects;
    }

    template <typename Material, typename Instance>
    void ShaderGroup<Material, Instance>::AddModel(const Ref<Model>& model, const std::any& material, const std::any& instance)
    {
        const Material& mat{ std::any_cast<Material>(material) };
        const Instance& inst{ std::any_cast<Instance>(instance) };

        auto modelIt{ std::find_if(m_Models.begin(), m_Models.end(), [&model](const PerModel& perModel) { return perModel.Model == model; }) };

        if (modelIt == m_Models.end())
        {
            PerModel perModel{};
            perModel.Model = model;
            perModel.PerMesh.resize(model->GetMeshesCount());

            m_Models.push_back(perModel);
            modelIt = m_Models.end() - 1;
        }

        auto materialFilter{ modelIt->PerMesh | std::views::filter([&mat](const PerMesh&) { return true; }) };

        std::ranges::for_each(materialFilter, [&inst](PerMesh& perMesh) { perMesh.Instances.push_back(inst); });
    }

    template <typename Material, typename Instance>
    void ShaderGroup<Material, Instance>::Render()
    {
        const auto& deviceContext{ DLEngine::D3D::GetDeviceContext4() };

        UpdateInstanceBuffer();

        m_InputLayout->Bind();
        
        m_VertexShader->Bind();
        m_PixelShader->Bind();

        deviceContext->RSSetState(m_RasterizerState.Get());

        m_InstanceBuffer->Bind(0u);

        uint32_t renderedInstances{ 0u };
        for (uint32_t modelIndex{ 0u }; modelIndex < m_Models.size(); ++modelIndex)
        {
            const auto& perModel{ m_Models[modelIndex] };

            perModel.Model->GetVertexBuffer()->Bind(1u);
            perModel.Model->GetIndexBuffer()->Bind();

            for (uint32_t meshIndex{ 0u }; meshIndex < perModel.PerMesh.size(); ++meshIndex)
            {
                const Mesh& srcMesh{ perModel.Model->GetMesh(meshIndex) };
                const auto& meshRange{ perModel.Model->GetMeshRange(meshIndex) };

                // TODO: Draw multiple instances of a mesh in one draw call
                uint32_t instancesCount{ static_cast<uint32_t>(perModel.PerMesh[meshIndex].Instances.size()) };
                for (uint32_t instanceIndex{ 0u }; instanceIndex < srcMesh.GetInstanceCount(); ++instanceIndex)
                {
                    UpdateAndSetPerDrawBuffer(modelIndex, meshIndex, instanceIndex);

                    DL_THROW_IF_D3D11(deviceContext->DrawIndexedInstanced(
                        meshRange.IndexCount,
                        instancesCount,
                        meshRange.IndexOffset,
                        meshRange.VertexOffset,
                        renderedInstances
                    ));
                }
                renderedInstances += instancesCount;
            }
        }
    }

    template <typename Material, typename Instance>
    void ShaderGroup<Material, Instance>::UpdateInstanceBuffer() const
    {
        uint32_t totalInstances{ 0u };
        for (const auto& perModel : m_Models)
            for (const auto& perMesh : perModel.PerMesh)
                totalInstances += static_cast<uint32_t>(perMesh.Instances.size());

        if (totalInstances == 0)
            return;

        m_InstanceBuffer->Resize(totalInstances);

        auto* instanceBufferPtr{ m_InstanceBuffer->Map() };

        uint32_t copiedCount{ 0u };
        for (const auto& perModel : m_Models)
        {
            for (const auto& perMesh : perModel.PerMesh)
            {
                for (const auto& instance : perMesh.Instances)
                {
                    instanceBufferPtr[copiedCount++] = std::any_cast<const Instance&>(instance);
                }
            }
        }

        m_InstanceBuffer->Unmap();
    }
}
