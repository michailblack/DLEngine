#include "dlpch.h"
#include "D3D11Texture.h"

#include "DLEngine/DirectX/D3D11Context.h"
#include "DLEngine/DirectX/D3D11Renderer.h"
    
#include <DirectXTex/DirectXTex.h>

#pragma comment(lib, "DirectXTex.lib")

namespace DLEngine
{
    namespace Utils
    {
        namespace
        {
            struct TextureData
            {
                DirectX::TexMetadata Metadata{};
                DirectX::DDSMetaData DDSMetadata{};
                Scope<DirectX::ScratchImage> ScratchImage{};
            };

            TextureData LoadTextureDataFromDDSFile(const std::filesystem::path& path)
            {
                TextureData data{};

                data.ScratchImage = CreateScope<DirectX::ScratchImage>();

                DL_THROW_IF_HR(DirectX::LoadFromDDSFileEx(
                    path.wstring().c_str(),
                    DirectX::DDS_FLAGS_NONE,
                    &data.Metadata,
                    &data.DDSMetadata,
                    *data.ScratchImage
                ));

                return data;
            }
        }

        void SaveD3D11TextureToDDSFile(
            const Microsoft::WRL::ComPtr<ID3D11Texture2D1>& texture,
            const std::filesystem::path& outputPath
        )
        {
            const auto& deviceContext{ D3D11Context::Get()->GetDeviceContext4() };
            const auto& device{ D3D11Context::Get()->GetDevice5() };

            DirectX::ScratchImage scratchImage{};
            DirectX::CaptureTexture(device.Get(), deviceContext.Get(), texture.Get(), scratchImage);

            D3D11_TEXTURE2D_DESC1 textureDesc{};
            texture->GetDesc1(&textureDesc);

            const auto& texFormat{ textureDesc.Format };

            DirectX::ScratchImage* imagePtr{ &scratchImage };

            DirectX::ScratchImage compressed;
            if (DirectX::IsCompressed(texFormat))
            {
                if (texFormat >= DXGI_FORMAT_BC6H_TYPELESS && texFormat <= DXGI_FORMAT_BC7_UNORM_SRGB)
                {
                    DL_THROW_IF_HR(DirectX::Compress(
                        device.Get(),
                        imagePtr->GetImages(),
                        imagePtr->GetImageCount(),
                        imagePtr->GetMetadata(),
                        static_cast<DXGI_FORMAT>(texFormat),
                        DirectX::TEX_COMPRESS_PARALLEL,
                        1.0f,
                        compressed
                    ));
                }
                else
                {
                    DL_THROW_IF_HR(DirectX::Compress(
                        imagePtr->GetImages(),
                        imagePtr->GetImageCount(),
                        imagePtr->GetMetadata(),
                        static_cast<DXGI_FORMAT>(texFormat),
                        DirectX::TEX_COMPRESS_PARALLEL,
                        1.0f,
                        compressed
                    ));
                }

                imagePtr = &compressed;
            }

            DirectX::SaveToDDSFile(
                imagePtr->GetImages(),
                imagePtr->GetImageCount(),
                imagePtr->GetMetadata(),
                DirectX::DDS_FLAGS(0),
                outputPath.wstring().c_str()
            );
        }

    }

    D3D11Texture2D::D3D11Texture2D(const TextureSpecification& specification)
        : m_Specification(specification)
    {
        D3D11_TEXTURE2D_DESC1 textureDesc{};
        textureDesc.Width = specification.Width;
        textureDesc.Height = specification.Height;
        textureDesc.MipLevels = specification.Mips;
        textureDesc.ArraySize = specification.Layers;
        textureDesc.Format = Utils::DXGIFormatFromTextureFormat(specification.Format);
        textureDesc.SampleDesc.Count = 1u;
        textureDesc.SampleDesc.Quality = 0u;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;

        switch (specification.Usage)
        {
        case TextureUsage::Texture:
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            break;
        case TextureUsage::Attachment:
            textureDesc.BindFlags = Utils::IsDepthFormat(specification.Format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
            break;
        case TextureUsage::TextureAttachment:
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (Utils::IsDepthFormat(specification.Format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET);
            break;
        case TextureUsage::None:
        default:
            DL_ASSERT(false);
            break;
        }

        textureDesc.CPUAccessFlags = 0u;
        textureDesc.MiscFlags = 0u;
        textureDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateTexture2D1(&textureDesc, nullptr, &m_D3D11Texture2D));
        DL_THROW_IF_HR(m_D3D11Texture2D->SetPrivateData(
            WKPDID_D3DDebugObjectName,
            static_cast<UINT>(specification.DebugName.size()),
            specification.DebugName.data()
        ));
    }

    D3D11Texture2D::D3D11Texture2D(const TextureSpecification& specification, const std::filesystem::path& path)
        : m_Specification(specification), m_Path(path)
    {
        const auto& [metadata, ddsMetadata, scratchImage] { Utils::LoadTextureDataFromDDSFile(path) };

        DL_ASSERT((metadata.miscFlags & DirectX::TEX_MISC_TEXTURECUBE) == 0,
            "Trying to create D3D11Texture2D object for texture {}, which is a cubemap", specification.DebugName
        );

        D3D11_TEXTURE2D_DESC1 textureDesc{};
        textureDesc.Width = static_cast<UINT>(metadata.width);
        textureDesc.Height = static_cast<UINT>(metadata.height);
        textureDesc.MipLevels = static_cast<UINT>(metadata.mipLevels);
        textureDesc.ArraySize = static_cast<UINT>(metadata.arraySize);
        textureDesc.Format = metadata.format;
        textureDesc.SampleDesc.Count = 1u;
        textureDesc.SampleDesc.Quality = 0u;

        switch (specification.Usage)
        {
        case TextureUsage::Texture:
            textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            break;
        case TextureUsage::Attachment:
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = Utils::IsDepthFormat(specification.Format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
            break;
        case TextureUsage::TextureAttachment:
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (Utils::IsDepthFormat(specification.Format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET);
            break;
        case TextureUsage::None:
        default:
            DL_ASSERT(false);
            break;
        }

        textureDesc.CPUAccessFlags = 0u;
        textureDesc.MiscFlags = 0u;
        textureDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        std::vector<D3D11_SUBRESOURCE_DATA> initData;
        const auto* images{ scratchImage->GetImages() };
        for (size_t i = 0; i < scratchImage->GetImageCount(); ++i)
        {
            D3D11_SUBRESOURCE_DATA data{};
            data.pSysMem = images[i].pixels;
            data.SysMemPitch = static_cast<UINT>(images[i].rowPitch);
            data.SysMemSlicePitch = static_cast<UINT>(images[i].slicePitch);
            initData.push_back(data);
        }

        m_Specification.Width = static_cast<uint32_t>(metadata.width);
        m_Specification.Height = static_cast<uint32_t>(metadata.height);
        m_Specification.Mips = static_cast<uint32_t>(metadata.mipLevels);
        m_Specification.Layers = static_cast<uint32_t>(metadata.arraySize);
        m_Specification.Format = Utils::TextureFormatFromDXGIFormat(metadata.format);

        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateTexture2D1(&textureDesc, initData.data(), &m_D3D11Texture2D));
        DL_THROW_IF_HR(m_D3D11Texture2D->SetPrivateData(
            WKPDID_D3DDebugObjectName,
            static_cast<UINT>(specification.DebugName.size()),
            specification.DebugName.data()
        ));
    }

    D3D11Texture2D::D3D11Texture2D(const D3D11Tex2D& d3d11Texture, const TextureSpecification& specification)
        : m_D3D11Texture2D(d3d11Texture), m_Specification(specification)
    {

    }

    D3D11Texture2D::D3D11SRV D3D11Texture2D::GetD3D11ShaderResourceView(const TextureViewSpecification& viewSpecification) const
    {
        const auto it{ m_D3D11ShaderResourceViewCache.find(viewSpecification) };
        if (it != m_D3D11ShaderResourceViewCache.end())
            return it->second;

        D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc{};
        srvDesc.Format = Utils::DXGIFormatFromTextureFormat(viewSpecification.Format);

        if (m_Specification.Layers == 0u)
        {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = viewSpecification.Subresource.BaseMip;
            srvDesc.Texture2D.MipLevels = viewSpecification.Subresource.MipsCount;
            srvDesc.Texture2D.PlaneSlice = 0u;
        }
        else
        {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            srvDesc.Texture2DArray.MostDetailedMip = viewSpecification.Subresource.BaseMip;
            srvDesc.Texture2DArray.MipLevels = viewSpecification.Subresource.MipsCount;
            srvDesc.Texture2DArray.FirstArraySlice = viewSpecification.Subresource.BaseLayer;
            srvDesc.Texture2DArray.ArraySize = viewSpecification.Subresource.LayersCount;
            srvDesc.Texture2DArray.PlaneSlice = 0u;
        }

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1> d3d11ShaderResourceView{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateShaderResourceView1(m_D3D11Texture2D.Get(), &srvDesc, &d3d11ShaderResourceView));

        std::string debugName{ m_Specification.DebugName + " SRV" };
        d3d11ShaderResourceView->SetPrivateData(
            WKPDID_D3DDebugObjectName,
            static_cast<UINT>(debugName.size()),
            debugName.data()
        );

        m_D3D11ShaderResourceViewCache[viewSpecification] = d3d11ShaderResourceView;

        return d3d11ShaderResourceView;
    }

    D3D11Texture2D::D3D11RTV D3D11Texture2D::GetD3D11RenderTargetView(const TextureViewSpecification& viewSpecification) const
    {
        const auto it{ m_D3D11RenderTargetViewCache.find(viewSpecification) };
        if (it != m_D3D11RenderTargetViewCache.end())
            return it->second;

        D3D11_RENDER_TARGET_VIEW_DESC1 rtvDesc{};
        rtvDesc.Format = Utils::DXGIFormatFromTextureFormat(viewSpecification.Format);
        
        if (m_Specification.Layers == 0u)
        {
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = viewSpecification.Subresource.BaseMip;
            rtvDesc.Texture2D.PlaneSlice = 0u;
        }
        else
        {
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice = viewSpecification.Subresource.BaseMip;
            rtvDesc.Texture2DArray.FirstArraySlice = viewSpecification.Subresource.BaseLayer;
            rtvDesc.Texture2DArray.ArraySize = viewSpecification.Subresource.LayersCount;
            rtvDesc.Texture2DArray.PlaneSlice = 0u;
        }

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> d3d11RenderTargetView{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateRenderTargetView1(m_D3D11Texture2D.Get(), &rtvDesc, &d3d11RenderTargetView));

        std::string debugName{ m_Specification.DebugName + " RTV" };
        d3d11RenderTargetView->SetPrivateData(
            WKPDID_D3DDebugObjectName,
            static_cast<UINT>(debugName.size()),
            debugName.data()
        );

        m_D3D11RenderTargetViewCache[viewSpecification] = d3d11RenderTargetView;

        return d3d11RenderTargetView;
    }

    D3D11Texture2D::D3D11DSV D3D11Texture2D::GetD3D11DepthStencilView(const TextureViewSpecification& viewSpecification) const
    {
        const auto it{ m_D3D11DepthStencilViewCache.find(viewSpecification) };
        if (it != m_D3D11DepthStencilViewCache.end())
            return it->second;

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = Utils::DXGIFormatFromTextureFormat(viewSpecification.Format);

        if (m_Specification.Layers == 0u)
        {
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = viewSpecification.Subresource.BaseMip;
        }
        else
        {
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.MipSlice = viewSpecification.Subresource.BaseMip;
            dsvDesc.Texture2DArray.FirstArraySlice = viewSpecification.Subresource.BaseLayer;
            dsvDesc.Texture2DArray.ArraySize = viewSpecification.Subresource.LayersCount;
        }

        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> d3d11DepthStencilView{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateDepthStencilView(m_D3D11Texture2D.Get(), &dsvDesc, &d3d11DepthStencilView));

        std::string debugName{ m_Specification.DebugName + " DSV" };
        d3d11DepthStencilView->SetPrivateData(
            WKPDID_D3DDebugObjectName,
            static_cast<UINT>(debugName.size()),
            debugName.data()
        );

        m_D3D11DepthStencilViewCache[viewSpecification] = d3d11DepthStencilView;

        return d3d11DepthStencilView;
    }

    D3D11TextureCube::D3D11TextureCube(const TextureSpecification& specification)
        : m_Specification(specification)
    {
        D3D11_TEXTURE2D_DESC1 textureDesc{};
        textureDesc.Width = specification.Width;
        textureDesc.Height = specification.Height;
        textureDesc.MipLevels = specification.Mips;
        textureDesc.ArraySize = specification.Layers * 6u;
        textureDesc.Format = Utils::DXGIFormatFromTextureFormat(specification.Format);
        textureDesc.SampleDesc.Count = 1u;
        textureDesc.SampleDesc.Quality = 0u;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;

        switch (specification.Usage)
        {
        case TextureUsage::Texture:
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            break;
        case TextureUsage::Attachment:
            textureDesc.BindFlags = Utils::IsDepthFormat(specification.Format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
            break;
        case TextureUsage::TextureAttachment:
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (Utils::IsDepthFormat(specification.Format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET);
            break;
        case TextureUsage::None:
        default:
            DL_ASSERT(false);
            break;
        }

        textureDesc.CPUAccessFlags = 0u;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        textureDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateTexture2D1(&textureDesc, nullptr, &m_D3D11Texture2D));
        DL_THROW_IF_HR(m_D3D11Texture2D->SetPrivateData(
            WKPDID_D3DDebugObjectName,
            static_cast<UINT>(specification.DebugName.size()),
            specification.DebugName.data()
        ));
    }

    D3D11TextureCube::D3D11TextureCube(const TextureSpecification& specification, const std::filesystem::path& path)
        : m_Specification(specification), m_Path(path)
    {
        const auto& [metadata, ddsMetadata, scratchImage] { Utils::LoadTextureDataFromDDSFile(path) };

        DL_ASSERT((metadata.miscFlags & DirectX::TEX_MISC_TEXTURECUBE) != 0,
            "Trying to create D3D11TextureCube object for texture {}, which is a 2D texture", specification.DebugName
        );

        D3D11_TEXTURE2D_DESC1 textureDesc{};
        textureDesc.Width = static_cast<UINT>(metadata.width);
        textureDesc.Height = static_cast<UINT>(metadata.height);
        textureDesc.MipLevels = static_cast<UINT>(metadata.mipLevels);
        textureDesc.ArraySize = static_cast<UINT>(metadata.arraySize);
        textureDesc.Format = metadata.format;
        textureDesc.SampleDesc.Count = 1u;
        textureDesc.SampleDesc.Quality = 0u;

        switch (specification.Usage)
        {
        case TextureUsage::Texture:
            textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            break;
        case TextureUsage::Attachment:
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = Utils::IsDepthFormat(specification.Format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
            break;
        case TextureUsage::TextureAttachment:
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (Utils::IsDepthFormat(specification.Format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET);
            break;
        case TextureUsage::None:
        default:
            DL_ASSERT(false);
            break;
        }

        textureDesc.CPUAccessFlags = 0u;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        textureDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        std::vector<D3D11_SUBRESOURCE_DATA> initData;
        const auto* images{ scratchImage->GetImages() };
        for (size_t i = 0; i < scratchImage->GetImageCount(); ++i)
        {
            D3D11_SUBRESOURCE_DATA data{};
            data.pSysMem = images[i].pixels;
            data.SysMemPitch = static_cast<UINT>(images[i].rowPitch);
            data.SysMemSlicePitch = static_cast<UINT>(images[i].slicePitch);
            initData.push_back(data);
        }

        m_Specification.Width = static_cast<uint32_t>(metadata.width);
        m_Specification.Height = static_cast<uint32_t>(metadata.height);
        m_Specification.Mips = static_cast<uint32_t>(metadata.mipLevels);
        m_Specification.Layers = static_cast<uint32_t>(metadata.arraySize) / 6u;
        m_Specification.Format = Utils::TextureFormatFromDXGIFormat(metadata.format);

        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateTexture2D1(&textureDesc, initData.data(), &m_D3D11Texture2D));
        DL_THROW_IF_HR(m_D3D11Texture2D->SetPrivateData(
            WKPDID_D3DDebugObjectName,
            static_cast<UINT>(specification.DebugName.size()),
            specification.DebugName.data()
        ));
    }

    D3D11TextureCube::D3D11SRV D3D11TextureCube::GetD3D11ShaderResourceView(const TextureViewSpecification& viewSpecification) const
    {
        const auto it{ m_D3D11ShaderResourceViewCache.find(viewSpecification) };
        if (it != m_D3D11ShaderResourceViewCache.end())
            return it->second;

        D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc{};
        srvDesc.Format = Utils::DXGIFormatFromTextureFormat(viewSpecification.Format);

        if (m_Specification.Layers == 1u)
        {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MostDetailedMip = viewSpecification.Subresource.BaseMip;
            srvDesc.TextureCube.MipLevels = viewSpecification.Subresource.MipsCount;
        }
        else
        {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
            srvDesc.TextureCubeArray.MostDetailedMip = viewSpecification.Subresource.BaseMip;
            srvDesc.TextureCubeArray.MipLevels = viewSpecification.Subresource.MipsCount;
            srvDesc.TextureCubeArray.First2DArrayFace = viewSpecification.Subresource.BaseLayer * 6u;
            srvDesc.TextureCubeArray.NumCubes = viewSpecification.Subresource.LayersCount;
        }

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1> d3d11ShaderResourceView{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateShaderResourceView1(m_D3D11Texture2D.Get(), &srvDesc, &d3d11ShaderResourceView));

        std::string debugName{ m_Specification.DebugName + " SRV" };
        d3d11ShaderResourceView->SetPrivateData(
            WKPDID_D3DDebugObjectName,
            static_cast<UINT>(debugName.size()),
            debugName.data()
        );

        m_D3D11ShaderResourceViewCache[viewSpecification] = d3d11ShaderResourceView;

        return d3d11ShaderResourceView;
    }

    D3D11TextureCube::D3D11RTV D3D11TextureCube::GetD3D11RenderTargetView(const TextureViewSpecification& viewSpecification) const
    {
        const auto it{ m_D3D11RenderTargetViewCache.find(viewSpecification) };
        if (it != m_D3D11RenderTargetViewCache.end())
            return it->second;

        D3D11_RENDER_TARGET_VIEW_DESC1 rtvDesc{};
        rtvDesc.Format = Utils::DXGIFormatFromTextureFormat(viewSpecification.Format);
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = viewSpecification.Subresource.BaseMip;
        rtvDesc.Texture2DArray.FirstArraySlice = viewSpecification.Subresource.BaseLayer * 6u;
        rtvDesc.Texture2DArray.ArraySize = viewSpecification.Subresource.LayersCount * 6u;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> d3d11RenderTargetView{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateRenderTargetView1(m_D3D11Texture2D.Get(), &rtvDesc, &d3d11RenderTargetView));

        std::string debugName{ m_Specification.DebugName + " RTV" };
        d3d11RenderTargetView->SetPrivateData(
            WKPDID_D3DDebugObjectName,
            static_cast<UINT>(debugName.size()),
            debugName.data()
        );

        m_D3D11RenderTargetViewCache[viewSpecification] = d3d11RenderTargetView;

        return d3d11RenderTargetView;
    }

    D3D11TextureCube::D3D11DSV D3D11TextureCube::GetD3D11DepthStencilView(const TextureViewSpecification& viewSpecification) const
    {
        const auto it{ m_D3D11DepthStencilViewCache.find(viewSpecification) };
        if (it != m_D3D11DepthStencilViewCache.end())
            return it->second;

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = Utils::DXGIFormatFromTextureFormat(viewSpecification.Format);
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.MipSlice = viewSpecification.Subresource.BaseMip;
        dsvDesc.Texture2DArray.FirstArraySlice = viewSpecification.Subresource.BaseLayer * 6u;
        dsvDesc.Texture2DArray.ArraySize = viewSpecification.Subresource.LayersCount * 6u;

        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> d3d11DepthStencilView{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateDepthStencilView(m_D3D11Texture2D.Get(), &dsvDesc, &d3d11DepthStencilView));

        std::string debugName{ m_Specification.DebugName + " DSV" };
        d3d11DepthStencilView->SetPrivateData(
            WKPDID_D3DDebugObjectName,
            static_cast<UINT>(debugName.size()),
            debugName.data()
        );

        m_D3D11DepthStencilViewCache[viewSpecification] = d3d11DepthStencilView;

        return d3d11DepthStencilView;
    }

}