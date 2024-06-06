#include "dlpch.h"
#include "MeshSystem.h"

#include "DLEngine/Core/Filesystem.h"

#include "DLEngine/DirectX/D3DStates.h"

#include <typeindex>

namespace DLEngine
{
    bool MeshSystem::ShadingGroupKey::operator<(const ShadingGroupKey& other) const noexcept
    {
        return std::tie(MaterialType, InstanceType) < std::tie(other.MaterialType, other.InstanceType);
    }

    void MeshSystem::Init()
    {
        InitNormalVisGroup();

        DL_LOG_INFO("Mesh System Initialized");
    }

    void MeshSystem::Render()
    {
        for (const auto& [key, shadingGroup] : m_ShadingGroups)
            shadingGroup->Render();
    }

    bool MeshSystem::Intersects(const Math::Ray& ray, IShadingGroup::IntersectInfo& outIntersectInfo) const noexcept
    {
        bool intersects{ false };
        for (const auto& [key, shadingGroup] : m_ShadingGroups)
            if (shadingGroup->Intersects(ray, outIntersectInfo))
                intersects = true;
        return intersects;
    }

    void MeshSystem::ToggleNormalVis() noexcept
    {
        ShadingGroupKey normalVisKey{
            .MaterialType = std::type_index{ typeid(NullMaterial) },
            .InstanceType = std::type_index{ typeid(NormalVisGroupInstance) }
        };

        auto normalVisShadingGroup{ m_ShadingGroups.find(normalVisKey) };

        DL_ASSERT(normalVisShadingGroup != m_ShadingGroups.end(), "NormalVis shading group does not exist");

        normalVisShadingGroup->second->ToggleRender();
    }

    void MeshSystem::InitNormalVisGroup()
    {
        ShadingGroupDesc normalVisGroupDesc{};

        normalVisGroupDesc.PipelineDesc.Topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

        normalVisGroupDesc.Name = "NormalVis";

        normalVisGroupDesc.InstanceBufferLayout = BufferLayout{
            { "TRANSFORM" , BufferLayout::ShaderDataType::Mat4  },
            { "_empty"    , BufferLayout::ShaderDataType::Float }
        };

        ShaderSpecification shaderSpec{};

        shaderSpec.Path = Filesystem::GetShaderDir() + L"NormalVis.hlsl";
        shaderSpec.EntryPoint = "mainVS";
        VertexShader vs{};
        vs.Create(shaderSpec);

        normalVisGroupDesc.PipelineDesc.VS = vs;

        shaderSpec.Path = Filesystem::GetShaderDir() + L"NormalVis.hlsl";
        shaderSpec.EntryPoint = "mainHS";
        HullShader hs{};
        hs.Create(shaderSpec);
        
        normalVisGroupDesc.PipelineDesc.HS = hs;

        shaderSpec.Path = Filesystem::GetShaderDir() + L"NormalVis.hlsl";
        shaderSpec.EntryPoint = "mainDS";
        DomainShader ds{};
        ds.Create(shaderSpec);

        normalVisGroupDesc.PipelineDesc.DS = ds;

        shaderSpec.Path = Filesystem::GetShaderDir() + L"NormalVis.hlsl";
        shaderSpec.EntryPoint = "mainGS";
        GeometryShader gs{};
        gs.Create(shaderSpec);

        normalVisGroupDesc.PipelineDesc.GS = gs;

        shaderSpec.Path = Filesystem::GetShaderDir() + L"NormalVis.hlsl";
        shaderSpec.EntryPoint = "mainPS";
        PixelShader ps{};
        ps.Create(shaderSpec);

        normalVisGroupDesc.PipelineDesc.PS = ps;

        normalVisGroupDesc.PipelineDesc.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::DEFAULT);

        normalVisGroupDesc.PipelineDesc.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEFAULT);

        normalVisGroupDesc.Render = false;

        CreateShadingGroup<NullMaterial, NormalVisGroupInstance>(normalVisGroupDesc);
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
