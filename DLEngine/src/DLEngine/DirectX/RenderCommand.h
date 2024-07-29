#pragma once
#include "DLEngine/DirectX/D3D.h"

#include "DLEngine/Math/Vec4.h"

enum class ShaderStage
{
    Vertex = 0u,
    Domain,
    Hull,
    Geometry,
    Pixel,
    Compute,

    All
};

namespace DLEngine
{
    class InputLayout;

    class VertexBuffer;
    class IndexBuffer;
    class ConstantBuffer;
    class StructuredBuffer;

    class VertexShader;
    class HullShader;
    class DomainShader;
    class GeometryShader;
    class PixelShader;

    class RasterizerState;
    class DepthStencilState;
    class SamplerState;

    struct PipelineState;

    class SwapChain;

    class Texture2D;

    class RenderCommand
    {
    public:
        static void SetRenderTargets(
            const std::initializer_list<Microsoft::WRL::ComPtr<ID3D11RenderTargetView1>>& RTVs = {},
            const Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& DSV = {}
        ) noexcept;

        static void SetShaderResources(
            uint32_t startSlot,
            ShaderStage stage,
            const std::initializer_list<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1>>& SRVs = {}
        ) noexcept;

        static void SetViewports(const std::initializer_list<D3D11_VIEWPORT>& viewports) noexcept;

        static void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) noexcept;

        static void SetInputLayout(const InputLayout& IL) noexcept;

        static void SetVertexBuffers(uint32_t startSlot, const std::initializer_list<VertexBuffer>& VBs) noexcept;
        static void SetIndexBuffer(const IndexBuffer& IB) noexcept;
        static void SetConstantBuffers(uint32_t startSlot, ShaderStage stage, const std::initializer_list<ConstantBuffer>& CBs) noexcept;

        static void SetVertexShader(const VertexShader& VS) noexcept;
        static void SetHullShader(const HullShader& HS) noexcept;
        static void SetDomainShader(const DomainShader& DS) noexcept;
        static void SetGeometryShader(const GeometryShader& GS) noexcept;
        static void SetPixelShader(const PixelShader& PS) noexcept;

        static void SetRasterizerState(const RasterizerState& RS) noexcept;
        static void SetDepthStencilState(const DepthStencilState& DSS) noexcept;
        static void SetSamplers(uint32_t startSlot, ShaderStage stage, const std::initializer_list<SamplerState>& SSs) noexcept;

        static void SetPipelineState(const PipelineState& PS) noexcept;

        static void ClearRenderTargetView(
            const Microsoft::WRL::ComPtr<ID3D11RenderTargetView1>& RTV,
            const Math::Vec4& color = Math::Vec4{ 1.0f, 0.0f, 1.0f, 1.0f }
        ) noexcept;
        static void ClearDepthStencilView(
            const Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& DSV,
            float depth = 0.0f, uint8_t stencil = 0
        ) noexcept;

        static void Draw(uint32_t vertexCount, uint32_t startVertexLocation = 0u);
        static void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0u, uint32_t baseVertexLocation = 0u);
        static void DrawInstancedIndexed(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation = 0u, uint32_t baseVertexLocation = 0u, uint32_t startInstanceLocation = 0u);

        static Texture2D GetBackBuffer(const SwapChain& swapChain);
    };
}
