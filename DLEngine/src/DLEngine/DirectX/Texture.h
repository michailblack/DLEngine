#pragma once
#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/View.h"

namespace DLEngine
{
    class Texture2D
    {
        friend class RenderCommand;
    public:
        void Create(const Microsoft::WRL::ComPtr<ID3D11Texture2D1>& handle) noexcept { m_Handle = handle; }
        void Create(const D3D11_TEXTURE2D_DESC1& desc);
        void Create(const D3D11_TEXTURE2D_DESC1& desc, const std::vector<D3D11_SUBRESOURCE_DATA>& data);

        void Reset() noexcept { m_Handle.Reset(); }

        D3D11_TEXTURE2D_DESC1 GetDesc() const;

    private:
        Microsoft::WRL::ComPtr<ID3D11Texture2D1> m_Handle{};
    };

    struct RTexture2D
    {
    public:
        void Create(const Texture2D& texture);

        void Reset() noexcept { m_Texture.Reset(); m_SRV.Reset(); }

        Texture2D GetTexture() const noexcept { return m_Texture; }
        ShaderResourceView GetSRV() const noexcept { return m_SRV; }

    private:
        Texture2D m_Texture{};
        ShaderResourceView m_SRV{};
    };

    struct WTexture2D
    {
    public:
        void Create(const Texture2D& texture);
        
        void Reset() noexcept { m_Texture.Reset(); m_RTV.Reset(); }
        
        Texture2D GetTexture() const noexcept { return m_Texture; }
        RenderTargetView GetRTV() const noexcept { return m_RTV; }

    private:
        Texture2D m_Texture{};
        RenderTargetView m_RTV{};
    };

    struct RWTexture2D
    {
    public:
        void Create(const Texture2D& texture);

        void Reset() noexcept { m_Texture.Reset(); m_RTV.Reset(); m_SRV.Reset(); }

        Texture2D GetTexture() const noexcept { return m_Texture; }
        RenderTargetView GetRTV() const noexcept { return m_RTV; }
        ShaderResourceView GetSRV() const noexcept { return m_SRV; }

    private:
        Texture2D m_Texture{};
        RenderTargetView m_RTV{};
        ShaderResourceView m_SRV{};
    };

    struct DTexture2D
    {
    public:
        void Create(const Texture2D& texture);

        void Reset() noexcept { m_Texture.Reset(); m_DSV.Reset(); }

        Texture2D GetTexture() const noexcept { return m_Texture; }
        DepthStencilView GetDSV() const noexcept { return m_DSV; }

    private:
        Texture2D m_Texture{};
        DepthStencilView m_DSV{};
    };

    struct RDTexture2D
    {
    public:
        void Create(const Texture2D& texture);

        void Reset() noexcept { m_Texture.Reset(); m_DSV.Reset(); m_SRV.Reset(); }

        Texture2D GetTexture() const noexcept { return m_Texture; }
        DepthStencilView GetDSV() const noexcept { return m_DSV; }
        ShaderResourceView GetSRV() const noexcept { return m_SRV; }

    private:
        Texture2D m_Texture{};
        DepthStencilView m_DSV{};
        ShaderResourceView m_SRV{};
    };
}

