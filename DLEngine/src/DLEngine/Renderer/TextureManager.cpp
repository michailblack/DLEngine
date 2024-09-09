#include "dlpch.h"
#include "TextureManager.h"

#include "DLEngine/Core/Filesystem.h"

#include <DirectXTex/DirectXTex.h>

#pragma comment(lib, "DirectXTex.lib")

enum class FileFormat
{
    NONE,
    PNG,
    TGA,
    HDR,
    BC1_LINEAR = DXGI_FORMAT_BC1_UNORM,			// RGB, 1 bit Alpha
    BC1_SRGB = DXGI_FORMAT_BC1_UNORM_SRGB,		// RGB, 1-bit Alpha, SRGB
    BC3_LINEAR = DXGI_FORMAT_BC3_UNORM,			// RGBA
    BC3_SRGB = DXGI_FORMAT_BC3_UNORM_SRGB,		// RGBA, SRGB
    BC4_UNSIGNED = DXGI_FORMAT_BC4_UNORM,		// GRAY, unsigned
    BC4_SIGNED = DXGI_FORMAT_BC4_SNORM,			// GRAY, signed
    BC5_UNSIGNED = DXGI_FORMAT_BC5_UNORM,		// RG, unsigned
    BC5_SIGNED = DXGI_FORMAT_BC5_SNORM,			// RG, signed
    BC6_UNSIGNED = DXGI_FORMAT_BC6H_UF16,		// RGB HDR, unsigned
    BC6_SIGNED = DXGI_FORMAT_BC6H_SF16,			// RGB HDR, signed
    BC7_LINEAR = DXGI_FORMAT_BC7_UNORM,			// RGBA Advanced
    BC7_SRGB = DXGI_FORMAT_BC7_UNORM_SRGB,		// RGBA Advanced, SRGB
};

namespace std
{
    template <>
    struct hash<DLEngine::Math::Vec4>
    {
        size_t operator()(const DLEngine::Math::Vec4& key) const noexcept
        {
            return std::hash<float>{}(key.x) ^ std::hash<float>{}(key.y) ^ std::hash<float>{}(key.z) ^ std::hash<float>{}(key.w);
        }
    };
}

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
        }

        namespace
        {
            TextureData LoadFromFile(const std::wstring& path)
            {
                TextureData data{};

                data.ScratchImage = CreateScope<DirectX::ScratchImage>();

                DL_THROW_IF_HR(DirectX::LoadFromDDSFileEx(
                    path.c_str(),
                    DirectX::DDS_FLAGS_NONE,
                    &data.Metadata,
                    &data.DDSMetadata,
                    *data.ScratchImage
                ));

                return data;
            }
        }
    }

    namespace
    {
        struct TextureManagerData
        {
            std::unordered_map<std::wstring, Texture2D> Textures2D;
            std::unordered_map<Math::Vec4, Texture2D> ValueTextures2D;
        } s_Data;
    }

    void TextureManager::Init()
    {
        DL_THROW_IF_HR(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

        DL_LOG_INFO("TextureManager Initialized");
    }

    Texture2D TextureManager::LoadTexture2D(const std::wstring& path)
    {
        if (Exists2D(path))
            return s_Data.Textures2D[path];

        const auto& [metadata, ddsMetadata, scratchImage] { Utils::LoadFromFile(path) };

        DL_ASSERT(
            metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D,
            "Texture must be 2D or a cube map"
        );

        D3D11_TEXTURE2D_DESC1 textureDesc{};
        textureDesc.Width = static_cast<UINT>(metadata.width);
        textureDesc.Height = static_cast<UINT>(metadata.height);
        textureDesc.MipLevels = static_cast<UINT>(metadata.mipLevels);
        textureDesc.ArraySize = static_cast<UINT>(metadata.arraySize);
        textureDesc.Format = metadata.format;
        textureDesc.SampleDesc.Count = 1u;
        textureDesc.SampleDesc.Quality = 0u;
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0u;
        textureDesc.MiscFlags = metadata.miscFlags;
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

        const std::string fileName{ std::filesystem::path{path}.stem().string() };

        Texture2D texture{};
        texture.Create(textureDesc, initData);
        texture.SetDebugName(fileName);

        const auto& [it, hasConstructed] { s_Data.Textures2D.emplace(std::make_pair(path, texture)) };

        return it->second;
    }

    Texture2D TextureManager::GetTexture2D(const std::wstring& path)
    {
        DL_ASSERT_NOINFO(Exists2D(path));

        return s_Data.Textures2D[path];
    }

    Texture2D TextureManager::GenerateValueTexture2D(Math::Vec4 value)
    {
        if (s_Data.ValueTextures2D.contains(value))
            return s_Data.ValueTextures2D[value];

        D3D11_TEXTURE2D_DESC1 textureDesc{};
        textureDesc.Width = 1u;
        textureDesc.Height = 1u;
        textureDesc.MipLevels = 1u;
        textureDesc.ArraySize = 1u;
        textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        textureDesc.SampleDesc.Count = 1u;
        textureDesc.SampleDesc.Quality = 0u;
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0u;
        textureDesc.MiscFlags = 0u;
        textureDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = &value;
        initData.SysMemPitch = sizeof(Math::Vec4);
        initData.SysMemSlicePitch = 0u;

        Texture2D texture{};
        texture.Create(textureDesc, std::vector{ initData });

        const auto& [it, hasConstructed] { s_Data.ValueTextures2D.emplace(std::make_pair(value, texture)) };

        return it->second;
    }

    void TextureManager::SaveToDDS(const Texture2D& texture, const std::wstring& name)
    {
        DirectX::ScratchImage scratchImage{};
        DirectX::CaptureTexture(D3D::GetDevice5().Get(), D3D::GetDeviceContext4().Get(), texture.GetHandle().Get(), scratchImage);

        const auto& texFormat{ texture.GetDesc().Format };

        DirectX::ScratchImage* imagePtr{ &scratchImage };

        DirectX::ScratchImage compressed;
        if (DirectX::IsCompressed(static_cast<DXGI_FORMAT>(texFormat)))
        {
            if (static_cast<DXGI_FORMAT>(FileFormat::BC6_UNSIGNED) <= texFormat && texFormat <= static_cast<DXGI_FORMAT>(FileFormat::BC7_SRGB))
            {
                DL_THROW_IF_HR(DirectX::Compress(
                    D3D::GetDevice5().Get(),
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

        const std::wstring file{ Filesystem::GetTextureDir() + L"saved/" + name + L".dds" };

        DirectX::SaveToDDSFile(imagePtr->GetImages(), imagePtr->GetImageCount(), imagePtr->GetMetadata(), DirectX::DDS_FLAGS(0), file.c_str());
    }

    bool TextureManager::Exists2D(const std::wstring& path)
    {
        return s_Data.Textures2D.contains(path);
    }
}
