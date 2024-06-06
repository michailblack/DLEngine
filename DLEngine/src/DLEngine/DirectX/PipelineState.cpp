#include "dlpch.h"
#include "PipelineState.h"

namespace DLEngine
{
    void PipelineState::Create(const PipelineStateDesc& spec) noexcept
    {
        m_Specification = spec;
    }

    void PipelineState::Bind() const noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        deviceContext->IASetPrimitiveTopology(m_Specification.Topology);

        m_Specification.Layout.Bind();
        
        m_Specification.VS.Bind();
        m_Specification.HS.Bind();
        m_Specification.DS.Bind();
        m_Specification.GS.Bind();
        m_Specification.PS.Bind();

        m_Specification.Rasterizer.Bind();
     
        m_Specification.DepthStencil.Bind();
    }
}
