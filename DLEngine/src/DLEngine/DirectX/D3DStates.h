#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class RasterizerState
    {
        friend class RenderCommand;
    public:
        void Create(const D3D11_RASTERIZER_DESC2& desc);

    private:
        Microsoft::WRL::ComPtr<ID3D11RasterizerState2> m_Handle{};
    };

    class DepthStencilState
    {
        friend class RenderCommand;
    public:
        void Create(const D3D11_DEPTH_STENCIL_DESC& desc);

    private:
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_Handle{};
    };

    class SamplerState
    {
        friend class RenderCommand;
    public:
        void Create(const D3D11_SAMPLER_DESC& desc);

    private:
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_Handle{};
    };

    enum class DepthStencilStates
    {
        DEFAULT,
        DEPTH_READ_ONLY,
        DEPTH_DISABLED,
    };

    enum class RasterizerStates
    {
        DEFAULT,
        CULL_NONE,
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
