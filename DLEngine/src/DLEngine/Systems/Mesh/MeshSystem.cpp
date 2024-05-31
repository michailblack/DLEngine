#include "dlpch.h"
#include "MeshSystem.h"

#include "DLEngine/Core/Filesystem.h"

#include "DLEngine/DirectX/DXStates.h"

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

    void MeshSystem::ToggleGroupRender() noexcept
    {
        for (const auto& [key, shadingGroup] : m_ShadingGroups)
            shadingGroup->ToggleRender();
    }

    void MeshSystem::InitNormalVisGroup()
    {
        ShadingGroupDesc normalVisGroupDesc{};

        normalVisGroupDesc.Name = "NormalVis";

        normalVisGroupDesc.InstanceBufferLayout = BufferLayout{
            { "TRANSFORM" , BufferLayout::ShaderDataType::Mat4  },
            { "_empty"    , BufferLayout::ShaderDataType::Float }
        };

        ShaderSpecification shaderSpec{};

        shaderSpec.Name = "NormalVis.vs";
        shaderSpec.Path = Filesystem::GetShaderDir() + "NormalVis.vs.hlsl";
        VertexShader vs{};
        vs.Create(shaderSpec);

        normalVisGroupDesc.PipelineDesc.VS = vs;

        shaderSpec.Name = "NormalVis.vs";
        shaderSpec.Path = Filesystem::GetShaderDir() + "NormalVis.ps.hlsl";
        PixelShader ps{};
        ps.Create(shaderSpec);

        normalVisGroupDesc.PipelineDesc.PS = ps;

        normalVisGroupDesc.PipelineDesc.DepthStencilState = DXStates::GetDepthStencilState(DepthStencilStates::Default);

        normalVisGroupDesc.PipelineDesc.RasterizerState = DXStates::GetRasterizerState(RasterizerStates::Default);

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
