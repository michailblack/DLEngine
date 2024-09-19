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

        D3D11Texture2D(const Ref<Texture2D>& other);

        // Needed for creating swap chain back buffer texture
        D3D11Texture2D(const D3D11Tex2D& d3d11Texture, const TextureSpecification& specification);

        void Resize(uint32_t width, uint32_t height, bool forceRecreate = false) override;

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
        uint32_t GetSamplesCount() const noexcept override { return m_Specification.Samples; }

        const TextureSpecification& GetSpecification() const noexcept override { return m_Specification; }

        const std::filesystem::path& GetPath() const noexcept override { return m_Path; }

        D3D11SRV GetD3D11ShaderResourceView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;
        D3D11RTV GetD3D11RenderTargetView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;
        D3D11DSV GetD3D11DepthStencilView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;
    
    private:
        void Create();
        void CreateFromFile();

    private:
        TextureSpecification m_Specification;
        std::filesystem::path m_Path;

        D3D11Tex2D m_D3D11Texture2D;

        mutable std::unordered_map<TextureViewSpecification, D3D11SRV, ByteBufferHash<TextureViewSpecification>> m_D3D11ShaderResourceViewCache;
        mutable std::unordered_map<TextureViewSpecification, D3D11RTV, ByteBufferHash<TextureViewSpecification>> m_D3D11RenderTargetViewCache;
        mutable std::unordered_map<TextureViewSpecification, D3D11DSV, ByteBufferHash<TextureViewSpecification>> m_D3D11DepthStencilViewCache;
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

        void Resize(uint32_t width, uint32_t height, bool forceRecreate = false) override;

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
        uint32_t GetSamplesCount() const noexcept override { return m_Specification.Samples; }

        const TextureSpecification& GetSpecification() const noexcept override { return m_Specification; }

        const std::filesystem::path& GetPath() const noexcept override { return m_Path; }

        D3D11SRV GetD3D11ShaderResourceView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;
        D3D11RTV GetD3D11RenderTargetView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;
        D3D11DSV GetD3D11DepthStencilView(const TextureViewSpecification& viewSpecification = TextureViewSpecification{}) const;

    private:
        void Create();
        void CreateFromFile();

    private:
        TextureSpecification m_Specification;
        std::filesystem::path m_Path;

        D3D11TexCube m_D3D11Texture2D;

        mutable std::unordered_map<TextureViewSpecification, D3D11SRV, ByteBufferHash<TextureViewSpecification>> m_D3D11ShaderResourceViewCache;
        mutable std::unordered_map<TextureViewSpecification, D3D11RTV, ByteBufferHash<TextureViewSpecification>> m_D3D11RenderTargetViewCache;
        mutable std::unordered_map<TextureViewSpecification, D3D11DSV, ByteBufferHash<TextureViewSpecification>> m_D3D11DepthStencilViewCache;
    };

    namespace Utils
    {
        inline DXGI_FORMAT DXGIFormatFromTextureFormat(TextureFormat format) noexcept
        {
            switch (format)
            {
            case TextureFormat::None:                  return DXGI_FORMAT_UNKNOWN;
                                                       
            case TextureFormat::R8_UNORM:              return DXGI_FORMAT_R8_UNORM;
            case TextureFormat::RG8_UNORM:             return DXGI_FORMAT_R8G8_UNORM;
            case TextureFormat::RGBA8_UNORM:           return DXGI_FORMAT_R8G8B8A8_UNORM;
                                                       
            case TextureFormat::RG16_FLOAT:            return DXGI_FORMAT_R16G16_FLOAT;
            case TextureFormat::RGBA16_FLOAT:          return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case TextureFormat::RGBA16_SNORM:          return DXGI_FORMAT_R16G16B16A16_SNORM;
                                                       
            case TextureFormat::RG32_FLOAT:            return DXGI_FORMAT_R32G32_FLOAT;
            case TextureFormat::RG32_UINT:             return DXGI_FORMAT_R32G32_UINT;
            case TextureFormat::RGBA32_FLOAT:          return DXGI_FORMAT_R32G32B32A32_FLOAT;
            
            case TextureFormat::R24_UNORM_X8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            
            case TextureFormat::BC1_UNORM:             return DXGI_FORMAT_BC1_UNORM;
            case TextureFormat::BC2_UNORM:             return DXGI_FORMAT_BC2_UNORM;
            case TextureFormat::BC3_UNORM:             return DXGI_FORMAT_BC3_UNORM;
            case TextureFormat::BC4_UNORM:             return DXGI_FORMAT_BC4_UNORM;
            case TextureFormat::BC4_SNORM:             return DXGI_FORMAT_BC4_SNORM;
            case TextureFormat::BC5_UNORM:             return DXGI_FORMAT_BC5_UNORM;
            case TextureFormat::BC5_SNORM:             return DXGI_FORMAT_BC5_SNORM;
            case TextureFormat::BC6H_UF16:             return DXGI_FORMAT_BC6H_UF16;
            case TextureFormat::BC6H_SF16:             return DXGI_FORMAT_BC6H_SF16;
            case TextureFormat::BC7_UNORM:             return DXGI_FORMAT_BC7_UNORM;
            case TextureFormat::BC7_UNORM_SRGB:        return DXGI_FORMAT_BC7_UNORM_SRGB;
            
            case TextureFormat::DEPTH24STENCIL8:       return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case TextureFormat::DEPTH_R24G8_TYPELESS:  return DXGI_FORMAT_R24G8_TYPELESS;
                                                       
            default: DL_ASSERT(false);                 return DXGI_FORMAT_UNKNOWN;
            }
        }

        inline TextureFormat TextureFormatFromDXGIFormat(DXGI_FORMAT format) noexcept
        {
            switch (format)
            {
            case DXGI_FORMAT_UNKNOWN:            return TextureFormat::None;
                                                 
            case DXGI_FORMAT_R8_UNORM:           return TextureFormat::R8_UNORM;
            case DXGI_FORMAT_R8G8_UNORM:         return TextureFormat::RG8_UNORM;
            case DXGI_FORMAT_R8G8B8A8_UNORM:     return TextureFormat::RGBA8_UNORM;

            case DXGI_FORMAT_R16G16_FLOAT:       return TextureFormat::RG16_FLOAT;
            case DXGI_FORMAT_R16G16B16A16_FLOAT: return TextureFormat::RGBA16_FLOAT;
            case DXGI_FORMAT_R16G16B16A16_SNORM: return TextureFormat::RGBA16_SNORM;

            case DXGI_FORMAT_R32G32_FLOAT:       return TextureFormat::RG32_FLOAT;
            case DXGI_FORMAT_R32G32_UINT:        return TextureFormat::RG32_UINT;
            case DXGI_FORMAT_R32G32B32A32_FLOAT: return TextureFormat::RGBA32_FLOAT;

            case DXGI_FORMAT_BC1_UNORM:          return TextureFormat::BC1_UNORM;
            case DXGI_FORMAT_BC2_UNORM:          return TextureFormat::BC2_UNORM;
            case DXGI_FORMAT_BC3_UNORM:          return TextureFormat::BC3_UNORM;
            case DXGI_FORMAT_BC4_UNORM:          return TextureFormat::BC4_UNORM;
            case DXGI_FORMAT_BC4_SNORM:          return TextureFormat::BC4_SNORM;
            case DXGI_FORMAT_BC5_UNORM:          return TextureFormat::BC5_UNORM;
            case DXGI_FORMAT_BC5_SNORM:          return TextureFormat::BC5_SNORM;
            case DXGI_FORMAT_BC6H_UF16:          return TextureFormat::BC6H_UF16;
            case DXGI_FORMAT_BC6H_SF16:          return TextureFormat::BC6H_SF16;
            case DXGI_FORMAT_BC7_UNORM:          return TextureFormat::BC7_UNORM;
            case DXGI_FORMAT_BC7_UNORM_SRGB:     return TextureFormat::BC7_UNORM_SRGB;

            case DXGI_FORMAT_D24_UNORM_S8_UINT:  return TextureFormat::DEPTH24STENCIL8;
                                                 
            default: DL_ASSERT(false);           return TextureFormat::None;
            }
        }

        static void SaveD3D11TextureToDDSFile(const Microsoft::WRL::ComPtr<ID3D11Texture2D1>& texture, const std::filesystem::path& outputPath);
    }
}