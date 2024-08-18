#include "dlpch.h"
#include "D3D11SwapChain.h"

#include "DLEngine/DirectX/D3D11Context.h"

namespace DLEngine
{
    D3D11SwapChain::D3D11SwapChain(
        Microsoft::WRL::ComPtr<ID3D11Device5> device5,
        Microsoft::WRL::ComPtr<IDXGIFactory7> factory7,
        HWND hWnd
    )
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

        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1{};

        DL_THROW_IF_HR(factory7->CreateSwapChainForHwnd(
            device5.Get(),
            hWnd,
            &swapChainDesk,
            nullptr,
            nullptr,
            &swapChain1
        ));

        DL_THROW_IF_HR(swapChain1.As(&m_D3D11SwapChain));
        DL_THROW_IF_HR(m_D3D11SwapChain->GetBuffer(0u, __uuidof(ID3D11Texture2D), &m_D3D11BackBuffer));
    }

    void D3D11SwapChain::Present() const
    {
        DXGI_PRESENT_PARAMETERS presentParams{};
        presentParams.DirtyRectsCount = 0u;
        presentParams.pDirtyRects = nullptr;
        presentParams.pScrollRect = nullptr;
        presentParams.pScrollOffset = nullptr;

        DL_THROW_IF_HR(m_D3D11SwapChain->Present1(1u, 0u, &presentParams));
    }

    void D3D11SwapChain::OnResize(uint32_t width, uint32_t height)
    {
        D3D11Context::Get()->GetDeviceContext4()->ClearState();
        DL_VERIFY(m_D3D11BackBuffer.Reset() == 0u, "Back buffer reference count is not zero before swap chain resizing");
        DL_THROW_IF_HR(m_D3D11SwapChain->ResizeBuffers(0u, width, height, DXGI_FORMAT_UNKNOWN, 0u));
        DL_THROW_IF_HR(m_D3D11SwapChain->GetBuffer(0u, __uuidof(ID3D11Texture2D1), &m_D3D11BackBuffer));
    }

}