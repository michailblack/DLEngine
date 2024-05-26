#pragma once
#include "DLEngine/Core/Base.h"

#include "DLEngine/DirectX/BufferLayout.h"
#include "DLEngine/DirectX/ConstantBuffer.h"
#include "DLEngine/DirectX/PipelineState.h"
#include "DLEngine/DirectX/Shaders.h"
#include "DLEngine/DirectX/VertexBuffer.h"

#include "DLEngine/Mesh/Model.h"

namespace DLEngine
{
    class IShadingGroup
    {
    public:
        virtual ~IShadingGroup() = default;

        virtual void Render() = 0;
    };

    struct ShadingGroupDesc
    {
        D3D_PRIMITIVE_TOPOLOGY Topology{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
        BufferLayout InstanceBufferLayout;
        ShaderSpecification VertexShaderSpec;
        ShaderSpecification PixelShaderSpec;
        ShaderSpecification DomainShaderSpec;
        ShaderSpecification HullShaderSpec;
        ShaderSpecification GeometryShaderSpec;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState2> RasterizerState;
    };

    template <typename TMaterial, typename TInstance>
    class ShadingGroup
        : public IShadingGroup
    {
    public:
        ShadingGroup(const ShadingGroupDesc& desc);

        void AddModel(const Ref<Model>& model, std::vector<TMaterial> meshMaterials, const TInstance& instance);
        void Render() override;

    private:
        void UpdateInstanceBuffer();

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
            Math::Mat4x4 MeshToModel;
            Math::Mat4x4 ModelToMesh;
        };

    private:
        std::vector<PerModel> m_Models;

        PipelineState m_PipelineState;
        VertexBuffer<TInstance, VertexBufferUsage::Dynamic> m_InstanceBuffer;
        ConstantBuffer<MeshInstance> m_MeshInstanceCB;
    };
}

#include "ShadingGroup.inl"
