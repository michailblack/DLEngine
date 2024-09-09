#include "dlpch.h"
#include "Texture.h"

#include "DLEngine/DirectX/RenderCommand.h"
#include "DLEngine/DirectX/SwapChain.h"

namespace DLEngine
{
    void Texture2D::Create(const D3D11_TEXTURE2D_DESC1& desc, const std::vector<D3D11_SUBRESOURCE_DATA>& data)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateTexture2D1(&desc, data.empty() ? nullptr : data.data(), &m_Handle));

        if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
            CreateSRV(desc);
        if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
            CreateRTV(desc);
        if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
            CreateDSV(desc);
    }

    void Texture2D::Create(const Microsoft::WRL::ComPtr<ID3D11Texture2D1>& handle)
    {
        m_Handle = handle;

        const auto& desc{ GetDesc() };

        if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
            CreateSRV(desc);
        if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
            CreateRTV(desc);
        if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
            CreateDSV(desc);
    }

    void Texture2D::SetDebugName(const std::string_view& name) const
    {
        DL_THROW_IF_HR(m_Handle->SetPrivateData(
            WKPDID_D3DDebugObjectName,
            static_cast<UINT>(name.size()),
            name.data()
        ));

        if (m_SRV.Get() != nullptr)
        {
            std::string debugName{ std::string(name) + "_SRV" };
            DL_THROW_IF_HR(m_SRV->SetPrivateData(
                WKPDID_D3DDebugObjectName,
                static_cast<UINT>(debugName.size()),
                debugName.c_str()));
        }

        if (m_DSV.Get() != nullptr)
        {
            std::string debugName{ std::string(name) + "_DSV" };
            DL_THROW_IF_HR(m_DSV->SetPrivateData(
                WKPDID_D3DDebugObjectName,
                static_cast<UINT>(debugName.size()),
                debugName.c_str()));
        }

        if (!m_RTVs.empty())
        {
            const auto& desc{ GetDesc() };
            for (uint32_t i{ 0u }; i < desc.MipLevels; ++i)
            {
                for (uint32_t j{ 0u }; j < desc.ArraySize; ++j)
                {
                    const uint32_t index{ D3D11CalcSubresource(i, j, desc.MipLevels) };
                    std::string debugName{ std::string(name) + "_RTV_MIP" + std::to_string(i) + "_ARR" + std::to_string(j) };
                    DL_THROW_IF_HR(m_RTVs[index]->SetPrivateData(
                        WKPDID_D3DDebugObjectName,
                        static_cast<UINT>(debugName.size()),
                        debugName.c_str()));
                }
            }
        }
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1> Texture2D::GetSRV() const noexcept
    {
        DL_ASSERT_NOINFO(m_SRV.Get() != nullptr);
        return m_SRV;
    }

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> Texture2D::GetRTV(uint32_t mipIndex, uint32_t arrayIndex) const noexcept
    {
        const auto& desc{ GetDesc() };
        const uint32_t index{ D3D11CalcSubresource(mipIndex, arrayIndex, desc.MipLevels) };

        DL_ASSERT_NOINFO(index < static_cast<uint32_t>(m_RTVs.size()));

        return m_RTVs[index];
    }

    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> Texture2D::GetDSV() const noexcept
    {
        DL_ASSERT_NOINFO(m_DSV.Get() != nullptr);
        return m_DSV;
    }

    D3D11_TEXTURE2D_DESC1 Texture2D::GetDesc() const noexcept
    {
        D3D11_TEXTURE2D_DESC1 desc;
        m_Handle->GetDesc1(&desc);
        return desc;
    }

    void Texture2D::CreateSRV(const D3D11_TEXTURE2D_DESC1& desc)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc{};
        srvDesc.Format = desc.Format;

        if (desc.ArraySize == 1u)
        {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0u;
            srvDesc.Texture2D.MipLevels = desc.MipLevels;
            srvDesc.Texture2D.PlaneSlice = 0u;
        }
        else
        {
            if (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
            {
                if (desc.ArraySize == 6u)
                {
                    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                    srvDesc.TextureCube.MostDetailedMip = 0u;
                    srvDesc.TextureCube.MipLevels = desc.MipLevels;
                }
                else
                {
                    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
                    srvDesc.TextureCubeArray.MostDetailedMip = 0u;
                    srvDesc.TextureCubeArray.MipLevels = desc.MipLevels;
                    srvDesc.TextureCubeArray.First2DArrayFace = 0u;
                    srvDesc.TextureCubeArray.NumCubes = desc.ArraySize / 6u;
                }
            }
            else
            {
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.MostDetailedMip = 0u;
                srvDesc.Texture2DArray.MipLevels = desc.MipLevels;
                srvDesc.Texture2DArray.FirstArraySlice = 0u;
                srvDesc.Texture2DArray.ArraySize = desc.ArraySize;
                srvDesc.Texture2DArray.PlaneSlice = 0u;
            }
        }

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateShaderResourceView1(m_Handle.Get(), &srvDesc, &m_SRV));
    }

    void Texture2D::CreateRTV(const D3D11_TEXTURE2D_DESC1& desc)
    {
        D3D11_RENDER_TARGET_VIEW_DESC1 rtvDesc{};
        rtvDesc.Format = desc.Format;

        if (desc.ArraySize == 1u)
        {
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.PlaneSlice = 0u;

            m_RTVs.resize(desc.MipLevels);
            for (uint32_t i{ 0u }; i < desc.MipLevels; ++i)
            {
                rtvDesc.Texture2D.MipSlice = i;
                DL_THROW_IF_HR(D3D::GetDevice5()->CreateRenderTargetView1(m_Handle.Get(), &rtvDesc, &m_RTVs[i]));
            }
        }
        else
        {
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.ArraySize = 1u;
            rtvDesc.Texture2DArray.PlaneSlice = 0u;

            m_RTVs.resize(desc.MipLevels * desc.ArraySize);
            for (uint32_t i{ 0u }; i < desc.MipLevels; ++i)
            {
                rtvDesc.Texture2DArray.MipSlice = i;
                for (uint32_t j{ 0u }; j < desc.ArraySize; ++j)
                {
                    rtvDesc.Texture2DArray.FirstArraySlice = j;
                    const uint32_t index{ D3D11CalcSubresource(i, j, desc.MipLevels) };
                    DL_THROW_IF_HR(D3D::GetDevice5()->CreateRenderTargetView1(
                        m_Handle.Get(),
                        &rtvDesc,
                        &m_RTVs[index]));
                }
            }
        }
    }

    void Texture2D::CreateDSV(const D3D11_TEXTURE2D_DESC1& desc)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = desc.Format;

        // Maybe add support for Texture2DArray in the future if needed
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

        dsvDesc.Flags = 0u;
        dsvDesc.Texture2D.MipSlice = 0u;

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateDepthStencilView(m_Handle.Get(), &dsvDesc, &m_DSV));
    }
}
