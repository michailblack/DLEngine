#pragma once
#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/Texture2D.h"

namespace DLEngine
{
    class SwapChain
    {
    public:
        void Create(HWND hWnd);

        void Resize(uint32_t width, uint32_t height);

        void Present() const;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> GetComPtr() const noexcept { return m_SwapChain; }
        const Texture2D& GetBackBuffer() const noexcept { return m_BackBuffer; }

    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain1> m_SwapChain;
        Texture2D m_BackBuffer;
    };
}

