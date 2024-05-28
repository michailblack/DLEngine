#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class Texture2D
    {
    public:
        void Create(D3D11_TEXTURE2D_DESC1 desc);
        void Create(const Microsoft::WRL::ComPtr<IDXGISwapChain1>& swapChain);

        Microsoft::WRL::ComPtr<ID3D11Texture2D1> GetComPtr() const noexcept { return m_Texture; }

        D3D11_TEXTURE2D_DESC1 GetDesc() const;

        void Reset() noexcept { m_Texture.Reset(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11Texture2D1> m_Texture;
    };
}

