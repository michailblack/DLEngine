#include "dlpch.h"
#include "TextureManager.h"

#include <DirectXTex/DirectXTex.h>

#pragma comment(lib, "DirectXTex.lib")

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
            std::unordered_map<std::wstring, RTexture2D> Textures2D;
            std::unordered_map<Math::Vec4, RTexture2D> ValueTextures2D;
        } s_Data;
    }

    void TextureManager::Init()
    {
        DL_THROW_IF_HR(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

        DL_LOG_INFO("TextureManager Initialized");
    }

    RTexture2D TextureManager::LoadTexture2D(const std::wstring& path)
    {
        if (Exists2D(path))
            return s_Data.Textures2D[path];

        const auto& [metadata, ddsMetadata, scratchImage]{ Utils::LoadFromFile(path) };

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

        Texture2D texture{};
        texture.Create(textureDesc, initData);

        RTexture2D resource{};
        resource.Create(texture);

        const auto& [it, hasConstructed]{ s_Data.Textures2D.emplace(std::make_pair(path, resource)) };

        return it->second;
    }

    RTexture2D TextureManager::GetTexture2D(const std::wstring& path)
    {
        DL_ASSERT_NOINFO(Exists2D(path));

        return s_Data.Textures2D[path];
    }

    RTexture2D TextureManager::GenerateValueTexture2D(Math::Vec4 value)
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
        texture.Create(textureDesc, std::vector { initData });

        RTexture2D resource{};
        resource.Create(texture);

        const auto& [it, hasConstructed]{ s_Data.ValueTextures2D.emplace(std::make_pair(value, resource)) };

        return it->second;
    }

    bool TextureManager::Exists2D(const std::wstring& path)
    {
        return s_Data.Textures2D.contains(path);
    }
}
