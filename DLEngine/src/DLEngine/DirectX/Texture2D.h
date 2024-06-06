#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class Texture2D
    {
    public:
        void Create(const D3D11_TEXTURE2D_DESC1& desc);
        void Create(const D3D11_TEXTURE2D_DESC1& desc, const std::vector<D3D11_SUBRESOURCE_DATA>& data);

        D3D11_TEXTURE2D_DESC1 GetDesc() const;

    public:
        Microsoft::WRL::ComPtr<ID3D11Texture2D1> Handle{};
    };
}

