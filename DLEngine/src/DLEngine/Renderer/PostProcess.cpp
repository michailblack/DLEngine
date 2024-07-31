#include "dlpch.h"
#include "PostProcess.h"

#include "DLEngine/Core/Filesystem.h"

#include "DLEngine/DirectX/View.h"

namespace DLEngine
{
    void PostProcess::Create()
    {
        PipelineStateDesc desc{};
        desc.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        ShaderSpecification shaderSpec{};
        shaderSpec.Path = Filesystem::GetShaderDir() + L"PostProcess.hlsl";

        shaderSpec.EntryPoint = "mainVS";
        VertexShader vs{};
        vs.Create(shaderSpec);
        desc.VS = vs;

        shaderSpec.EntryPoint = "mainPS";
        PixelShader ps{};
        ps.Create(shaderSpec);
        desc.PS = ps;

        desc.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::DEFAULT);
        desc.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEPTH_DISABLED);

        m_PipelineState.Create(desc);

        m_SettingsCB.Create();
    }

    void PostProcess::SetSettings(const PostProcessSettings& settings) noexcept
    {
        m_CurrentSettings = settings;
        m_SettingsCB.Set(&m_CurrentSettings, 1u);
    }

    void PostProcess::Resolve(const ShaderResourceView& src, const RenderTargetView& dst) const
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        m_PipelineState.Bind();
        src.Bind(0u, BIND_PS);
        m_SettingsCB.Bind(9u, BIND_PS);
        
        auto* renderTarget{ static_cast<ID3D11RenderTargetView*>(dst.Handle.Get()) };
        deviceContext->OMSetRenderTargets(1u, &renderTarget, nullptr);

        dst.Clear(Math::Vec4{ 1.0f, 0.0f, 1.0f, 1.0f });

        DL_THROW_IF_D3D11(deviceContext->Draw(3u, 0u));

        ID3D11ShaderResourceView* nullSRV{ nullptr };
        deviceContext->PSSetShaderResources(0u, 1u, &nullSRV);
    }
}
