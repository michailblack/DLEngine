#include "dlpch.h"
#include "Texture2D.h"

namespace DLEngine
{
    void Texture2D::Create(D3D11_TEXTURE2D_DESC1 desc)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateTexture2D1(&desc, nullptr, &m_Texture));
    }

    void Texture2D::Create(const Microsoft::WRL::ComPtr<IDXGISwapChain1>& swapChain)
    {
        DL_THROW_IF_HR(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D1), &m_Texture));
    }

    D3D11_TEXTURE2D_DESC1 Texture2D::GetDesc() const
    {
        D3D11_TEXTURE2D_DESC1 desc;
        m_Texture->GetDesc1(&desc);
        return desc;
    }

}
