#include "dlpch.h"
#include "Texture.h"

#include "DLEngine/DirectX/SwapChain.h"

namespace DLEngine
{
    void Texture2D::Create(const D3D11_TEXTURE2D_DESC1& desc)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateTexture2D1(&desc, nullptr, &Handle));
    }

    void Texture2D::Create(const D3D11_TEXTURE2D_DESC1& desc, const std::vector<D3D11_SUBRESOURCE_DATA>& data)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateTexture2D1(&desc, data.data(), &Handle));
    }

    D3D11_TEXTURE2D_DESC1 Texture2D::GetDesc() const
    {
        D3D11_TEXTURE2D_DESC1 desc;
        Handle->GetDesc1(&desc);
        return desc;
    }
}
