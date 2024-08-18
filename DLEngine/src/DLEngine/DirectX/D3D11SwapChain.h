#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace DLEngine
{
    class D3D11SwapChain
    {
    public:
        D3D11SwapChain(Microsoft::WRL::ComPtr<ID3D11Device5> device5, Microsoft::WRL::ComPtr<IDXGIFactory7> factory7, HWND hWnd);

        void Present() const;

        void OnResize(uint32_t width, uint32_t height);

        Microsoft::WRL::ComPtr<IDXGISwapChain4> GetD3D11SwapChain() const noexcept { return m_D3D11SwapChain; }
        Microsoft::WRL::ComPtr<ID3D11Texture2D1> GetD3D11BackBuffer() const noexcept { return m_D3D11BackBuffer; }

    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_D3D11SwapChain;
        Microsoft::WRL::ComPtr<ID3D11Texture2D1> m_D3D11BackBuffer;
    };
}

