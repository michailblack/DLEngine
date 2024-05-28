#include "dlpch.h"
#include "PipelineState.h"

namespace DLEngine
{
    void PipelineState::Create(const PipelineStateDesc& spec) noexcept
    {
        m_PrimitiveTopology = spec.Topology;
        m_InputLayout = spec.Layout;
        m_VertexShader = spec.VS;
        m_PixelShader = spec.PS;
        m_DomainShader = spec.DS;
        m_HullShader = spec.HS;
        m_GeometryShader = spec.GS;
        m_DepthStencilState = spec.DepthStencilState;
        m_RasterizerState = spec.RasterizerState;
    }

    void PipelineState::Bind() const noexcept
    {
        D3D::GetDeviceContext4()->IASetPrimitiveTopology(m_PrimitiveTopology);
        D3D::GetDeviceContext4()->IASetInputLayout(m_InputLayout.Get());
        D3D::GetDeviceContext4()->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
        D3D::GetDeviceContext4()->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
        D3D::GetDeviceContext4()->DSSetShader(m_DomainShader.Get(), nullptr, 0u);
        D3D::GetDeviceContext4()->HSSetShader(m_HullShader.Get(), nullptr, 0u);
        D3D::GetDeviceContext4()->GSSetShader(m_GeometryShader.Get(), nullptr, 0u);
        D3D::GetDeviceContext4()->OMSetDepthStencilState(m_DepthStencilState.Get(), 0u);
        D3D::GetDeviceContext4()->RSSetState(m_RasterizerState.Get());
    }
}
