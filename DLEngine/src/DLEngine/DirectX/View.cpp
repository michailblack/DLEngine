#include "dlpch.h"
#include "View.h"

#include "DLEngine/DirectX/Texture2D.h"

namespace DLEngine
{
    void RenderTargetView::Create(const Texture2D& texture)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateRenderTargetView1(texture.Handle.Get(), nullptr, &Handle));
    }

    void RenderTargetView::Clear(const Math::Vec4& color) const noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        
        deviceContext->ClearRenderTargetView(Handle.Get(), color.data());
    }

    void DepthStencilView::Create(const Texture2D& texture)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateDepthStencilView(texture.Handle.Get(), nullptr, &Handle));
    }

    void DepthStencilView::Clear(float depth, uint8_t stencil) const noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        
        deviceContext->ClearDepthStencilView(Handle.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }

    void ShaderResourceView::Create(const Texture2D& texture)
    {
        const auto& desc{ texture.GetDesc() };

        D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc{};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = desc.MipLevels - 1u;
        srvDesc.Texture2D.MipLevels = static_cast<UINT>(-1);
        srvDesc.Texture2D.PlaneSlice = 0u;

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateShaderResourceView1(texture.Handle.Get(), &srvDesc, &Handle));
    }

    void ShaderResourceView::Bind(uint32_t slot, uint8_t shaderBindFlags) const noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        auto* const srv{ static_cast<ID3D11ShaderResourceView*>(Handle.Get()) };

        if (shaderBindFlags & BIND_VS)
            deviceContext->VSSetShaderResources(slot, 1u, &srv);

        if (shaderBindFlags & BIND_HS)
            deviceContext->HSSetShaderResources(slot, 1u, &srv);

        if (shaderBindFlags & BIND_DS)
            deviceContext->DSSetShaderResources(slot, 1u, &srv);

        if (shaderBindFlags & BIND_GS)
            deviceContext->GSSetShaderResources(slot, 1u, &srv);

        if (shaderBindFlags & BIND_PS)
            deviceContext->PSSetShaderResources(slot, 1u, &srv);
    }
}
