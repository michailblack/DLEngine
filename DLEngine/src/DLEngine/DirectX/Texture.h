#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class Texture2D
    {
    public:
        void Create(const D3D11_TEXTURE2D_DESC1& desc, const std::vector<D3D11_SUBRESOURCE_DATA>& data = {});
        void Create(const Microsoft::WRL::ComPtr<ID3D11Texture2D1>& handle);

        void SetDebugName(const std::string_view& name) const;

        void Reset() noexcept { m_Handle.Reset(); m_SRV.Reset(); m_RTVs.clear(); m_DSV.Reset(); }

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1> GetSRV() const noexcept;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> GetRTV(uint32_t mipIndex = 0u, uint32_t arrayIndex = 0u) const noexcept;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetDSV() const noexcept;
        Microsoft::WRL::ComPtr<ID3D11Texture2D1> GetHandle() const noexcept { return m_Handle; }

        D3D11_TEXTURE2D_DESC1 GetDesc() const noexcept;

        bool operator==(const Texture2D& other) const noexcept { return m_Handle == other.m_Handle; }

    private:
        void CreateSRV(const D3D11_TEXTURE2D_DESC1& desc);
        void CreateRTV(const D3D11_TEXTURE2D_DESC1& desc);
        void CreateDSV(const D3D11_TEXTURE2D_DESC1& desc);

    private:
        Microsoft::WRL::ComPtr<ID3D11Texture2D1> m_Handle{};
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1> m_SRV{};
        std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView1>> m_RTVs{};
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DSV{};
    };
}
