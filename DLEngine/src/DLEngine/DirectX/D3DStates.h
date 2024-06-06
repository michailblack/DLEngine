#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class RasterizerState
    {
    public:
        void Create(const D3D11_RASTERIZER_DESC2& desc);

        void Bind() const noexcept;

    public:
        Microsoft::WRL::ComPtr<ID3D11RasterizerState2> Handle{};
    };

    class DepthStencilState
    {
    public:
        void Create(const D3D11_DEPTH_STENCIL_DESC& desc);

        void Bind() const noexcept;

    public:
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> Handle{};
    };

    class SamplerState
    {
    public:
        void Create(const D3D11_SAMPLER_DESC& desc);

        void Bind(uint32_t slot, uint8_t shaderBindFlags) const noexcept;

    public:
        Microsoft::WRL::ComPtr<ID3D11SamplerState> Handle{};
    };

    enum class DepthStencilStates
    {
        DEFAULT,
    };

    enum class RasterizerStates
    {
        DEFAULT,
    };

    enum class SamplerStates
    {
        POINT_WRAP,
        POINT_CLAMP,

        TRILINEAR_WRAP,
        TRILINEAR_CLAMP,

        ANISOTROPIC_8_WRAP,
        ANISOTROPIC_8_CLAMP,
    };

    class D3DStates
    {
    public:
        static RasterizerState GetRasterizerState(const D3D11_RASTERIZER_DESC2& desc);
        static RasterizerState GetRasterizerState(RasterizerStates state);

        static DepthStencilState GetDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc);
        static DepthStencilState GetDepthStencilState(DepthStencilStates state);

        static SamplerState GetSamplerState(const D3D11_SAMPLER_DESC& desc);
        static SamplerState GetSamplerState(SamplerStates state);
    };
}
