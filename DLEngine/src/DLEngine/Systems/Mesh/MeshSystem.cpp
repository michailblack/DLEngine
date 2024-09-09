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
        InitHologramGroup();
        InitTextureOnlyGroup();
        InitEmissionGroup();
        InitLitGroup();

        DL_LOG_INFO("Mesh System Initialized");
    }

    void MeshSystem::Render()
    {
        for (const auto& shadingGroup : m_ShadingGroups | std::views::values)
            shadingGroup->Render();
    }

    bool MeshSystem::Intersects(const Math::Ray& ray, IShadingGroup::IntersectInfo& outIntersectInfo) const noexcept
    {
        bool intersects{ false };
        for (const auto& shadingGroup : m_ShadingGroups | std::views::values)
            if (shadingGroup->Intersects(ray, outIntersectInfo))
                intersects = true;
        return intersects;
    }

    void MeshSystem::ToggleNormalVis() noexcept
    {
        ShadingGroupKey normalVisKey{
            .MaterialType = std::type_index{ typeid(ShadingGroupStruct::Material::Null) },
            .InstanceType = std::type_index{ typeid(ShadingGroupStruct::Instance::Null) }
        };

        auto normalVisShadingGroup{ m_ShadingGroups.find(normalVisKey) };

        DL_ASSERT(normalVisShadingGroup != m_ShadingGroups.end(), "NormalVis shading group does not exist");

        normalVisShadingGroup->second->ToggleRender();
    }

    void MeshSystem::InitNormalVisGroup()
    {
        ShadingGroupDesc normalVisGroupDesc{};

        normalVisGroupDesc.PipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

        normalVisGroupDesc.Name = "NormalVis";

        normalVisGroupDesc.InstanceBufferLayout = BufferLayout{
            { "TRANSFORM_INDEX", BufferLayout::ShaderDataType::Uint }
        };

        ShaderSpecification shaderSpec{};

        shaderSpec.Path = Filesystem::GetShaderDir() + L"NormalVis.hlsl";
        shaderSpec.EntryPoint = "mainVS";
        VertexShader vs{};
        vs.Create(shaderSpec);

        normalVisGroupDesc.PipelineState.VS = vs;

        shaderSpec.Path = Filesystem::GetShaderDir() + L"NormalVis.hlsl";
        shaderSpec.EntryPoint = "mainHS";
        HullShader hs{};
        hs.Create(shaderSpec);
        
        normalVisGroupDesc.PipelineState.HS = hs;

        shaderSpec.Path = Filesystem::GetShaderDir() + L"NormalVis.hlsl";
        shaderSpec.EntryPoint = "mainDS";
        DomainShader ds{};
        ds.Create(shaderSpec);

        normalVisGroupDesc.PipelineState.DS = ds;

        shaderSpec.Path = Filesystem::GetShaderDir() + L"NormalVis.hlsl";
        shaderSpec.EntryPoint = "mainGS";
        GeometryShader gs{};
        gs.Create(shaderSpec);

        normalVisGroupDesc.PipelineState.GS = gs;

        shaderSpec.Path = Filesystem::GetShaderDir() + L"NormalVis.hlsl";
        shaderSpec.EntryPoint = "mainPS";
        PixelShader ps{};
        ps.Create(shaderSpec);

        normalVisGroupDesc.PipelineState.PS = ps;

        normalVisGroupDesc.PipelineState.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::DEFAULT);

        normalVisGroupDesc.PipelineState.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEFAULT);

        normalVisGroupDesc.Render = false;

        CreateShadingGroup<ShadingGroupStruct::Material::Null, ShadingGroupStruct::Instance::Null>(normalVisGroupDesc);
    }

    void MeshSystem::InitHologramGroup()
    {
        ShadingGroupDesc hologramGroupDesc{};

        hologramGroupDesc.Name = "Hologram";

        hologramGroupDesc.PipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

        hologramGroupDesc.InstanceBufferLayout = BufferLayout{
            { "BASE_COLOR"     , BufferLayout::ShaderDataType::Float3 },
            { "ADD_COLOR"      , BufferLayout::ShaderDataType::Float3 },
            { "TRANSFORM_INDEX", BufferLayout::ShaderDataType::Uint }
        };

        ShaderSpecification shaderSpec{};
        shaderSpec.Path = Filesystem::GetShaderDir() + L"Hologram.hlsl";

        shaderSpec.EntryPoint = "mainVS";
        VertexShader vs{};
        vs.Create(shaderSpec);
        hologramGroupDesc.PipelineState.VS = vs;

        shaderSpec.EntryPoint = "mainPS";
        PixelShader ps{};
        ps.Create(shaderSpec);
        hologramGroupDesc.PipelineState.PS = ps;

        shaderSpec.EntryPoint = "mainHS";
        HullShader hs{};
        hs.Create(shaderSpec);
        hologramGroupDesc.PipelineState.HS = hs;

        shaderSpec.EntryPoint = "mainDS";
        DomainShader ds{};
        ds.Create(shaderSpec);
        hologramGroupDesc.PipelineState.DS = ds;
        
        shaderSpec.EntryPoint = "mainGS";
        GeometryShader gs{};
        gs.Create(shaderSpec);
        hologramGroupDesc.PipelineState.GS = gs;

        hologramGroupDesc.PipelineState.DepthStencil = D3DStates::GetDepthStencilState(DLEngine::DepthStencilStates::DEFAULT);
        hologramGroupDesc.PipelineState.Rasterizer = D3DStates::GetRasterizerState(DLEngine::RasterizerStates::DEFAULT);

        CreateShadingGroup<ShadingGroupStruct::Material::Null, ShadingGroupStruct::Instance::Hologram>(hologramGroupDesc);
    }

    void MeshSystem::InitTextureOnlyGroup()
    {
        ShadingGroupDesc textureOnlyGroupDesc{};

        textureOnlyGroupDesc.Name = "TextureOnly";

        textureOnlyGroupDesc.PipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        textureOnlyGroupDesc.InstanceBufferLayout = BufferLayout{
            { "TRANSFORM_INDEX", BufferLayout::ShaderDataType::Uint }
        };

        ShaderSpecification shaderSpec{};
        shaderSpec.Path = Filesystem::GetShaderDir() + L"TextureOnly.hlsl";

        shaderSpec.EntryPoint = "mainVS";
        VertexShader vs{};
        vs.Create(shaderSpec);
        textureOnlyGroupDesc.PipelineState.VS = vs;

        shaderSpec.EntryPoint = "mainPS";
        PixelShader ps{};
        ps.Create(shaderSpec);
        textureOnlyGroupDesc.PipelineState.PS = ps;

        textureOnlyGroupDesc.PipelineState.DepthStencil = D3DStates::GetDepthStencilState(DLEngine::DepthStencilStates::DEFAULT);
        textureOnlyGroupDesc.PipelineState.Rasterizer = D3DStates::GetRasterizerState(DLEngine::RasterizerStates::DEFAULT);

        CreateShadingGroup<ShadingGroupStruct::Material::TextureOnly, ShadingGroupStruct::Instance::Null>(textureOnlyGroupDesc);
    }

    void MeshSystem::InitEmissionGroup()
    {
        ShadingGroupDesc emissionGroupDesc{};

        emissionGroupDesc.Name = "Emission";

        emissionGroupDesc.PipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        emissionGroupDesc.InstanceBufferLayout = BufferLayout{
            { "POINT_LIGHT_INDEX", BufferLayout::ShaderDataType::Uint },
            { "TRANSFORM_INDEX"  , BufferLayout::ShaderDataType::Uint }
        };

        ShaderSpecification shaderSpec{};
        shaderSpec.Path = Filesystem::GetShaderDir() + L"Emission.hlsl";

        shaderSpec.EntryPoint = "mainVS";
        VertexShader vs{};
        vs.Create(shaderSpec);
        emissionGroupDesc.PipelineState.VS = vs;

        shaderSpec.EntryPoint = "mainPS";
        PixelShader ps{};
        ps.Create(shaderSpec);
        emissionGroupDesc.PipelineState.PS = ps;

        emissionGroupDesc.PipelineState.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEFAULT);
        emissionGroupDesc.PipelineState.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::DEFAULT);

        CreateShadingGroup<ShadingGroupStruct::Material::Null, ShadingGroupStruct::Instance::Emission>(emissionGroupDesc);
    }

    void MeshSystem::InitLitGroup()
    {
        ShadingGroupDesc litGroupDesc{};

        litGroupDesc.Name = "Lit";

        litGroupDesc.PipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        litGroupDesc.InstanceBufferLayout = BufferLayout{
            { "TRANSFORM_INDEX", BufferLayout::ShaderDataType::Uint }
        };

        ShaderSpecification shaderSpec{};
        shaderSpec.Path = Filesystem::GetShaderDir() + L"Lit.hlsl";

        shaderSpec.EntryPoint = "mainVS";
        VertexShader vs{};
        vs.Create(shaderSpec);
        litGroupDesc.PipelineState.VS = vs;

        shaderSpec.EntryPoint = "mainPS";
        PixelShader ps{};
        ps.Create(shaderSpec);
        litGroupDesc.PipelineState.PS = ps;

        litGroupDesc.PipelineState.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEFAULT);
        litGroupDesc.PipelineState.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::DEFAULT);

        CreateShadingGroup<ShadingGroupStruct::Material::Lit, ShadingGroupStruct::Instance::Null>(litGroupDesc);
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
