#include "dlpch.h"
#include "PostProcess.h"

#include "DLEngine/Core/Filesystem.h"
#include "DLEngine/DirectX/RenderCommand.h"

#include "DLEngine/DirectX/View.h"

namespace DLEngine
{
    void PostProcess::Create()
    {
        m_PipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        ShaderSpecification shaderSpec{};
        shaderSpec.Path = Filesystem::GetShaderDir() + L"PostProcess.hlsl";

        shaderSpec.EntryPoint = "mainVS";
        VertexShader vs{};
        vs.Create(shaderSpec);
        m_PipelineState.VS = vs;

        shaderSpec.EntryPoint = "mainPS";
        PixelShader ps{};
        ps.Create(shaderSpec);
        m_PipelineState.PS = ps;

        m_PipelineState.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::DEFAULT);
        m_PipelineState.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEPTH_DISABLED);

        m_SettingsCB.Create(sizeof(PostProcessSettings));
    }

    void PostProcess::SetSettings(const PostProcessSettings& settings) noexcept
    {
        m_CurrentSettings = settings;
        m_SettingsCB.Set(&m_CurrentSettings);
    }

    void PostProcess::Resolve(const ShaderResourceView& src, const RenderTargetView& dst) const
    {
        RenderCommand::SetPipelineState(m_PipelineState);
        RenderCommand::SetShaderResources(0u, ShaderStage::Pixel, { src });
        RenderCommand::SetConstantBuffers(9u, ShaderStage::Pixel, { m_SettingsCB });
        
        RenderCommand::SetRenderTargets({ dst }, DepthStencilView{});
        RenderCommand::ClearRenderTargetView(dst);

        RenderCommand::Draw(3u);

        RenderCommand::SetShaderResources(0u, ShaderStage::Pixel, { ShaderResourceView{} });
    }
}
