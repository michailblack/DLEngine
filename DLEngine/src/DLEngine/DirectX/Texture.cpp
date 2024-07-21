#include "dlpch.h"
#include "Texture.h"

#include "DLEngine/DirectX/RenderCommand.h"
#include "DLEngine/DirectX/SwapChain.h"

namespace DLEngine
{
    void Texture2D::Create(const D3D11_TEXTURE2D_DESC1& desc)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateTexture2D1(&desc, nullptr, &m_Handle));
    }

    void Texture2D::Create(const D3D11_TEXTURE2D_DESC1& desc, const std::vector<D3D11_SUBRESOURCE_DATA>& data)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateTexture2D1(&desc, data.data(), &m_Handle));
    }

    D3D11_TEXTURE2D_DESC1 Texture2D::GetDesc() const
    {
        D3D11_TEXTURE2D_DESC1 desc;
        m_Handle->GetDesc1(&desc);
        return desc;
    }

    void RTexture2D::Create(const Texture2D& texture)
    {
        m_Texture = texture;
        m_SRV = RenderCommand::CreateShaderResourceView(texture);
    }

    void WTexture2D::Create(const Texture2D& texture)
    {
        m_Texture = texture;
        m_RTV = RenderCommand::CreateRenderTargetView(texture);
    }

    void RWTexture2D::Create(const Texture2D& texture)
    {
        m_Texture = texture;
        m_RTV = RenderCommand::CreateRenderTargetView(texture);
        m_SRV = RenderCommand::CreateShaderResourceView(texture);
    }

    void DTexture2D::Create(const Texture2D& texture)
    {
        m_Texture = texture;
        m_DSV = RenderCommand::CreateDepthStencilView(texture);
    }

    void RDTexture2D::Create(const Texture2D& texture)
    {
        m_Texture = texture;
        m_DSV = RenderCommand::CreateDepthStencilView(texture);
        m_SRV = RenderCommand::CreateShaderResourceView(texture);
    }

}
