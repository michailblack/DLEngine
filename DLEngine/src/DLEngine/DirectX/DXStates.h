#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    enum class DepthStencilStates
    {
        Default,
    };

    enum class RasterizerStates
    {
        Default,
    };

    class DXStates
    {
    public:
        static void Init();

        static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> GetDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc);
        static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> GetDepthStencilState(DepthStencilStates state);

        static Microsoft::WRL::ComPtr<ID3D11RasterizerState2> GetRasterizerState(const D3D11_RASTERIZER_DESC2& desc);
        static Microsoft::WRL::ComPtr<ID3D11RasterizerState2> GetRasterizerState(RasterizerStates state);
    };
}
