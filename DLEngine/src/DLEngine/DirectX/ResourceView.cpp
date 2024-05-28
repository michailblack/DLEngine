#include "dlpch.h"
#include "ResourceView.h"

namespace DLEngine
{
    void RenderTargetView::Create(const Microsoft::WRL::ComPtr<ID3D11Texture2D1>& texture)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateRenderTargetView1(texture.Get(), nullptr, &m_RenderTargetView));
    }

    void RenderTargetView::Clear(const Math::Vec4& color) const noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        
        deviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), color.data());
    }

    void DepthStencilView::Create(const Microsoft::WRL::ComPtr<ID3D11Texture2D1>& texture)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateDepthStencilView(texture.Get(), nullptr, &m_DepthStencilView));
    }

    void DepthStencilView::Clear(float depth, uint8_t stencil) const noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        
        deviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }

}
