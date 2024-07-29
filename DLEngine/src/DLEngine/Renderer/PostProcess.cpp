#include "dlpch.h"
#include "PostProcess.h"

#include "DLEngine/Core/Filesystem.h"

#include "DLEngine/DirectX/RenderCommand.h"
#include "DLEngine/DirectX/Texture.h"

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
    }

    void PostProcess::Resolve(const Texture2D& src, const Texture2D& dst) const
    {
        RenderCommand::SetPipelineState(m_PipelineState);
        RenderCommand::SetShaderResources(0u, ShaderStage::Pixel, { src.GetSRV() });

        RenderCommand::SetRenderTargets({ dst.GetRTV() });
        RenderCommand::ClearRenderTargetView(dst.GetRTV());

        RenderCommand::Draw(3u);

        RenderCommand::SetShaderResources(0u, ShaderStage::Pixel);
    }
}
