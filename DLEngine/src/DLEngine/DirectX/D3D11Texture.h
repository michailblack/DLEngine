#pragma once
#include "DLEngine/Core/Assert.h"

#include "DLEngine/Renderer/Texture.h"

#include "DLEngine/Math/Vec2.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class D3D11Texture2D : public Texture2D
    {
        using D3D11Tex2D   = Microsoft::WRL::ComPtr<ID3D11Texture2D1>;
        using D3D11SRV     = Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1>;
        using D3D11RTV     = Microsoft::WRL::ComPtr<ID3D11RenderTargetView1>;
        using D3D11DSV     = Microsoft::WRL::ComPtr<ID3D11DepthStencilView>;

    public:
        D3D11Texture2D(const TextureSpecification& specification);
        D3D11Texture2D(const TextureSpecification& specification, const std::filesystem::path& path);

        // Needed for creating swap chain back buffer texture
        D3D11Texture2D(const D3D11Tex2D& d3d11Texture, const TextureSpecification& specification);

        uint32_t GetWidth() const noexcept override { return m_Specification.Width; }
        uint32_t GetHeight() const noexcept override { return m_Specification.Height; }
        Math::Vec2 GetSize() const noexcept override
        {
            return Math::Vec2{
                static_cast<float>(m_Specification.Width),
                static_cast<float>(m_Specification.Height)
            };
        }

        uint32_t GetMipsCount() const noexcept override { return m_Specification.Mips; }
        uint32_t GetLayersCount() const noexcept override { return m_Specification.Layers; }

        const TextureSpecification& GetSpecification() const noexcept override { return m_Specification; }

        const std::filesystem::path& GetPath() const noexcept override { return m_Path; }

        D3D11SRV GetD3D11ShaderResourceView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;
        D3D11RTV GetD3D11RenderTargetView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;
        D3D11DSV GetD3D11DepthStencilView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;

    private:
        TextureSpecification m_Specification;
        std::filesystem::path m_Path;

        D3D11Tex2D m_D3D11Texture2D;

        mutable std::unordered_map<TextureViewSpecification, D3D11SRV, TextureViewSpecificationHash> m_D3D11ShaderResourceViewCache;
        mutable std::unordered_map<TextureViewSpecification, D3D11RTV, TextureViewSpecificationHash> m_D3D11RenderTargetViewCache;
        mutable std::unordered_map<TextureViewSpecification, D3D11DSV, TextureViewSpecificationHash> m_D3D11DepthStencilViewCache;
    };

    class D3D11TextureCube : public TextureCube
    {
        using D3D11TexCube = Microsoft::WRL::ComPtr<ID3D11Texture2D1>;
        using D3D11SRV     = Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1>;
        using D3D11RTV     = Microsoft::WRL::ComPtr<ID3D11RenderTargetView1>;
        using D3D11DSV     = Microsoft::WRL::ComPtr<ID3D11DepthStencilView>;

    public:
        D3D11TextureCube(const TextureSpecification& specification);
        D3D11TextureCube(const TextureSpecification& specification, const std::filesystem::path& path);

        uint32_t GetWidth() const noexcept { return m_Specification.Width; }
        uint32_t GetHeight() const noexcept override { return m_Specification.Height; }
        Math::Vec2 GetSize() const noexcept override
        {
            return Math::Vec2{
                static_cast<float>(m_Specification.Width),
                static_cast<float>(m_Specification.Height)
            };
        }

        uint32_t GetMipsCount() const noexcept override { return m_Specification.Mips; }
        uint32_t GetLayersCount() const noexcept override { return m_Specification.Layers; }

        const TextureSpecification& GetSpecification() const noexcept override { return m_Specification; }

        const std::filesystem::path& GetPath() const noexcept override { return m_Path; }

        D3D11SRV GetD3D11ShaderResourceView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;
        D3D11RTV GetD3D11RenderTargetView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;
        D3D11DSV GetD3D11DepthStencilView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;

    private:
        TextureSpecification m_Specification;
        std::filesystem::path m_Path;

        D3D11TexCube m_D3D11Texture2D;

        mutable std::unordered_map<TextureViewSpecification, D3D11SRV, TextureViewSpecificationHash> m_D3D11ShaderResourceViewCache;
        mutable std::unordered_map<TextureViewSpecification, D3D11RTV, TextureViewSpecificationHash> m_D3D11RenderTargetViewCache;
        mutable std::unordered_map<TextureViewSpecification, D3D11DSV, TextureViewSpecificationHash> m_D3D11DepthStencilViewCache;
    };

    namespace Utils
    {
        inline DXGI_FORMAT DXGIFormatFromTextureFormat(TextureFormat format) noexcept
        {
            switch (format)
            {
            case TextureFormat::None:            return DXGI_FORMAT_UNKNOWN;
            case TextureFormat::RG16F:           return DXGI_FORMAT_R16G16_FLOAT;
            case TextureFormat::RG32F:           return DXGI_FORMAT_R32G32_FLOAT;
            case TextureFormat::RGBA:            return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::RGBA16F:         return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case TextureFormat::RGBA32F:         return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case TextureFormat::R24X8:           return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case TextureFormat::BC1U:            return DXGI_FORMAT_BC1_UNORM;
            case TextureFormat::BC2U:            return DXGI_FORMAT_BC2_UNORM;
            case TextureFormat::BC3U:            return DXGI_FORMAT_BC3_UNORM;
            case TextureFormat::BC4U:            return DXGI_FORMAT_BC4_UNORM;
            case TextureFormat::BC4S:            return DXGI_FORMAT_BC4_SNORM;
            case TextureFormat::BC5U:            return DXGI_FORMAT_BC5_UNORM;
            case TextureFormat::BC5S:            return DXGI_FORMAT_BC5_SNORM;
            case TextureFormat::BC6H:            return DXGI_FORMAT_BC6H_UF16;
            case TextureFormat::BC6S:            return DXGI_FORMAT_BC6H_SF16;
            case TextureFormat::BC7U:            return DXGI_FORMAT_BC7_UNORM;
            case TextureFormat::DEPTH24STENCIL8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case TextureFormat::DEPTH_R24G8T:    return DXGI_FORMAT_R24G8_TYPELESS;
            default: DL_ASSERT(false);           return DXGI_FORMAT_UNKNOWN;
            }
        }

        inline TextureFormat TextureFormatFromDXGIFormat(DXGI_FORMAT format) noexcept
        {
            switch (format)
            {
            case DXGI_FORMAT_R16G16_FLOAT:          return TextureFormat::RG16F;
            case DXGI_FORMAT_R32G32_FLOAT:          return TextureFormat::RG32F;
            case DXGI_FORMAT_R8G8B8A8_UNORM:        return TextureFormat::RGBA;
            case DXGI_FORMAT_R16G16B16A16_FLOAT:    return TextureFormat::RGBA16F;
            case DXGI_FORMAT_R32G32B32A32_FLOAT:    return TextureFormat::RGBA32F;
            case DXGI_FORMAT_BC1_UNORM:             return TextureFormat::BC1U;
            case DXGI_FORMAT_BC2_UNORM:             return TextureFormat::BC2U;
            case DXGI_FORMAT_BC3_UNORM:             return TextureFormat::BC3U;
            case DXGI_FORMAT_BC4_UNORM:             return TextureFormat::BC4U;
            case DXGI_FORMAT_BC4_SNORM:             return TextureFormat::BC4S;
            case DXGI_FORMAT_BC5_UNORM:             return TextureFormat::BC5U;
            case DXGI_FORMAT_BC5_SNORM:             return TextureFormat::BC5S;
            case DXGI_FORMAT_BC6H_UF16:             return TextureFormat::BC6H;
            case DXGI_FORMAT_BC6H_SF16:             return TextureFormat::BC6S;
            case DXGI_FORMAT_BC7_UNORM:             return TextureFormat::BC7U;
            case DXGI_FORMAT_D24_UNORM_S8_UINT:     return TextureFormat::DEPTH24STENCIL8;
            case DXGI_FORMAT_UNKNOWN:               return TextureFormat::None;
            default: DL_ASSERT(false);              return TextureFormat::None;
            }
        }

        static void SaveD3D11TextureToDDSFile(const Microsoft::WRL::ComPtr<ID3D11Texture2D1>& texture, const std::filesystem::path& outputPath);
    }
}