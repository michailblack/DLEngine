#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class Texture2D;

    class SwapChain
    {
        friend class RenderCommand;
    public:
        void Create(HWND hWnd);

        void Resize(uint32_t width, uint32_t height) const;

        void Present() const;

    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain1> m_Handle{};
    };
}

