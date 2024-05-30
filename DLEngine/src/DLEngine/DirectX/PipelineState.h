#pragma once
#include "DLEngine/DirectX/D3D.h"

#include "DLEngine/DirectX/InputLayout.h"
#include "DLEngine/DirectX/Shaders.h"

namespace DLEngine
{
    struct PipelineStateDesc
    {
        VertexShader VS{};
        PixelShader PS{};
        DomainShader DS{};
        HullShader HS{};
        GeometryShader GS{};
        InputLayout Layout{};
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11RasterizerState2> RasterizerState{ nullptr };
        D3D_PRIMITIVE_TOPOLOGY Topology{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
    };

    class PipelineState
    {
    public:
        void Create(const PipelineStateDesc& spec) noexcept;
        void Bind() const noexcept;

    private:
        PipelineStateDesc m_Specification;
    };
}

