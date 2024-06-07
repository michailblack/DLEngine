#include "dlpch.h"
#include "SwapChain.h"

#include "DLEngine/DirectX/Texture.h"

namespace DLEngine
{    
    void SwapChain::Create(HWND hWnd)
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesk{};
        swapChainDesk.Width = 0u;
        swapChainDesk.Height = 0u;
        swapChainDesk.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesk.Stereo = FALSE;
        swapChainDesk.SampleDesc.Count = 1;
        swapChainDesk.SampleDesc.Quality = 0;
        swapChainDesk.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesk.BufferCount = 2;
        swapChainDesk.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesk.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesk.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesk.Flags = 0;

        DL_THROW_IF_HR(D3D::GetFactory7()->CreateSwapChainForHwnd(
            D3D::GetDevice5().Get(),
            hWnd,
            &swapChainDesk,
            nullptr,
            nullptr,
            &Handle
        ));
    }

    void SwapChain::Resize(uint32_t width, uint32_t height)
    {
        DL_THROW_IF_HR(Handle->ResizeBuffers(0u, width, height, DXGI_FORMAT_UNKNOWN, 0));
    }

    void SwapChain::Present() const
    {
        DL_THROW_IF_HR(Handle->Present(1, 0));
    }

    Texture2D SwapChain::GetBackBuffer() const
    {
        Texture2D texture{};
        DL_THROW_IF_HR(Handle->GetBuffer(0, __uuidof(ID3D11Texture2D1), &texture.Handle));
        return texture;
    }
}
