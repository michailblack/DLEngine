#pragma once
#include "DLEngine/Renderer/Pipeline.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    namespace Utils
    {
        inline D3D11_PRIMITIVE_TOPOLOGY D3D11TopologyFromPrimitiveTopology(PrimitiveTopology topology) noexcept
        {
            switch (topology)
            {
            case PrimitiveTopology::TrianglesList: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case PrimitiveTopology::TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case PrimitiveTopology::None:
            default:
                DL_ASSERT(false, "Unknown PrimitiveTopology");
                return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        }
    }

    class D3D11Pipeline : public Pipeline
    {
    public:
        D3D11Pipeline(const PipelineSpecification& specificaton);

        void SetFramebuffer(const Ref<Framebuffer>& framebuffer) noexcept override { m_Specification.TargetFramebuffer = framebuffer; }

        const PipelineSpecification& GetSpecification() const noexcept override { return m_Specification; }

        D3D11_PRIMITIVE_TOPOLOGY GetD3D11PrimitiveTopology() const noexcept { return Utils::D3D11TopologyFromPrimitiveTopology(m_Specification.Topology); }

    private:
        PipelineSpecification m_Specification;
    };
}