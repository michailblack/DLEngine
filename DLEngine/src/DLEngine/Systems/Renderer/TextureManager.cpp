#include "dlpch.h"
#include "TextureManager.h"

#include <DirectXTex/DirectXTex.h>

#pragma comment(lib, "DirectXTex.lib")

namespace DLEngine
{
    namespace
    {
        struct TextureManagerData
        {
            std::unordered_map<std::wstring, Texture2D> Textures;
        } s_Data;
    }

    void TextureManager::Init()
    {
        DL_THROW_IF_HR(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

        DL_LOG_INFO("TextureManager Initialized");
    }

    Texture2D TextureManager::Load(const std::wstring& path)
    {
        if (Exists(path))
            return s_Data.Textures[path];

        Texture2D texture{ LoadFromFile(path) };

        const auto& [it, hasConstructed]{ s_Data.Textures.emplace(std::make_pair(path, texture)) };

        return it->second;
    }

    Texture2D TextureManager::Get(const std::wstring& path)
    {
        DL_ASSERT_NOINFO(Exists(path));

        return s_Data.Textures[path];
    }

    bool TextureManager::Exists(const std::wstring& path)
    {
        return s_Data.Textures.contains(path);
    }

    Texture2D TextureManager::LoadFromFile(const std::wstring& path)
    {
        DirectX::TexMetadata metadata{};
        DirectX::DDSMetaData ddsMetadata{};

        auto scratchImage{ CreateScope<DirectX::ScratchImage>() };

        DL_THROW_IF_HR(DirectX::LoadFromDDSFileEx(
            path.c_str(),
            DirectX::DDS_FLAGS_NONE,
            &metadata,
            &ddsMetadata,
            *scratchImage
        ));

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

        Texture2D texture{};
        texture.Create(textureDesc, initData);

        return texture;
    }
}
