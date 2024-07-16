#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class Texture2D;

    class SwapChain
    {
    public:
        void Create(HWND hWnd);

        void Resize(uint32_t width, uint32_t height);

        void Present() const;

        Texture2D GetBackBuffer() const;

    public:
        Microsoft::WRL::ComPtr<IDXGISwapChain1> Handle;
    };
}

