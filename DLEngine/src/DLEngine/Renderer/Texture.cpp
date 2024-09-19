#include "dlpch.h"
#include "Texture.h"

#include "DLEngine/Core/Application.h"

#include "DLEngine/DirectX/D3D11Texture.h"

namespace DLEngine
{

    Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification)
    {
        return CreateRef<D3D11Texture2D>(specification);
    }

    Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification, const std::filesystem::path& path)
    {
        return CreateRef<D3D11Texture2D>(specification, path);
    }

    Ref<Texture2D> Texture2D::Copy(const Ref<Texture2D>& other)
    {
        return CreateRef<D3D11Texture2D>(other);
    }

    Ref<TextureCube> TextureCube::Create(const TextureSpecification& specification)
    {
        return CreateRef<D3D11TextureCube>(specification);
    }

    Ref<TextureCube> TextureCube::Create(const TextureSpecification& specification, const std::filesystem::path& path)
    {
        return CreateRef<D3D11TextureCube>(specification, path);
    }

    const std::filesystem::path Texture::GetTextureDirectoryPath() noexcept
    {
        return Application::Get().GetWorkingDir() / "assets\\textures\\";
    }

    void TextureLibrary::Add(const Ref<Texture>& texture)
    {
        DL_ASSERT(!m_Textures.contains(texture->GetPath()), "Texture [{0}] already added in the texture library", texture->GetSpecification().DebugName);
        m_Textures[texture->GetPath()] = texture;
    }

    Ref<Texture2D> TextureLibrary::LoadTexture2D(const TextureSpecification& specification, const std::filesystem::path& path)
    {
        Ref<Texture2D> texture{ Texture2D::Create(specification, path) };
        Add(texture);

        return texture;
    }

    Ref<TextureCube> TextureLibrary::LoadTextureCube(const TextureSpecification& specification, const std::filesystem::path& path)
    {
        Ref<TextureCube> texture{ TextureCube::Create(specification, path) };
        Add(texture);

        return texture;
    }

    Ref<Texture> TextureLibrary::Get(const std::filesystem::path& path) const
    {
        DL_ASSERT(m_Textures.contains(path));
        return m_Textures.at(path);
    }
}