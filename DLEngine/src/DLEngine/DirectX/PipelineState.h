#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    struct PipelineStateDesc
    {
        D3D_PRIMITIVE_TOPOLOGY Topology{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
        Microsoft::WRL::ComPtr<ID3D11InputLayout> Layout{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11VertexShader> VS{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11PixelShader> PS{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11DomainShader> DS{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11HullShader> HS{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11GeometryShader> GS{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11RasterizerState2> RasterizerState{ nullptr };
    };

    class PipelineState
    {
    public:
        void Create(const PipelineStateDesc& spec) noexcept;
        void Bind() const noexcept;

    private:
        D3D_PRIMITIVE_TOPOLOGY m_PrimitiveTopology;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
        Microsoft::WRL::ComPtr<ID3D11DomainShader> m_DomainShader;
        Microsoft::WRL::ComPtr<ID3D11HullShader> m_HullShader;
        Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_GeometryShader;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState2> m_RasterizerState;
    };
}

