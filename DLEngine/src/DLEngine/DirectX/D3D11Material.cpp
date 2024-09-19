#include "dlpch.h"
#include "D3D11Material.h"

#include "DLEngine/DirectX/D3D11Shader.h"

namespace DLEngine
{

    D3D11Material::D3D11Material(const Ref<Shader>& shader, const std::string& name) noexcept
        : m_Name(name), m_Shader(shader)
    {
        SetShaderStageFlags();
    }

    D3D11Material::D3D11Material(const Ref<Material>& material, const std::string& name) noexcept
        : m_Name(name), m_Shader(material->GetShader())
    {
        const auto& d3d11Material{ AsRef<D3D11Material>(material) };

        m_ConstantBuffers = d3d11Material->m_ConstantBuffers;
        m_ConstantBuffersShaderStages = d3d11Material->m_ConstantBuffersShaderStages;

        m_Texture2Ds = d3d11Material->m_Texture2Ds;
        m_TextureCubes = d3d11Material->m_TextureCubes;
        m_TextureViews = d3d11Material->m_TextureViews;
        m_TextureShaderStages = d3d11Material->m_TextureShaderStages;
    }

    void D3D11Material::Set(const std::string& name, const Ref<ConstantBuffer>& buffer) noexcept
    {
        DL_ASSERT(buffer, "Trying to set an empty constant buffer in the material [{0}] for [{1}]", m_Name, name);

        const auto shaderConstantBuffer{ FindConstantBuffer(name) };

        m_ConstantBuffers[shaderConstantBuffer.BindPoint] = buffer;
    }

    void D3D11Material::Set(const std::string& name, const Ref<Texture2D>& texture) noexcept
    {
        DL_ASSERT(texture, "Trying to set an empty texture in the material [{0}] for [{1}]", m_Name, name);

        const auto shaderTexture{ FindTexture(name) };

        DL_ASSERT(shaderTexture.Type == ShaderTextureType::Texture2D || shaderTexture.Type == ShaderTextureType::Texture2DArray,
            "Texture with name [{0}] is not a 2D texture in the material [{1}]", name, m_Name
        );

        m_Texture2Ds[shaderTexture.BindPoint] = texture;
        m_TextureViews[shaderTexture.BindPoint] = TextureViewSpecification{};
    }

    void D3D11Material::Set(const std::string& name, const Ref<TextureCube>& texture) noexcept
    {
        DL_ASSERT(texture, "Trying to set an empty texture in the material [{0}] for [{1}]", m_Name, name)

        const auto shaderTexture{ FindTexture(name) };

        DL_ASSERT(shaderTexture.Type == ShaderTextureType::TextureCube || shaderTexture.Type == ShaderTextureType::TextureCubeArray,
            "Texture with name [{0}] is not a 2D texture in the material [{1}]", name, m_Name
        );

        m_TextureCubes[shaderTexture.BindPoint] = texture;
        m_TextureViews[shaderTexture.BindPoint] = TextureViewSpecification{};
    }

    void D3D11Material::SetTextureView(const std::string& name, const TextureViewSpecification& view) noexcept
    {
        const auto shaderTexture{ FindTexture(name) };

        m_TextureViews[shaderTexture.BindPoint] = view;
    }

    Ref<ConstantBuffer> D3D11Material::GetConstantBuffer(const std::string& name) const noexcept
    {
        const auto shaderConstantBuffer{ FindConstantBuffer(name) };

        DL_ASSERT(m_ConstantBuffers.find(shaderConstantBuffer.BindPoint) != m_ConstantBuffers.end(),
            "Constant buffer with name [{0}] is not set in the material [{1}]", name, m_Name
        );

        return m_ConstantBuffers.at(shaderConstantBuffer.BindPoint);
    }

    Ref<Texture2D> D3D11Material::GetTexture2D(const std::string& name) const noexcept
    {
        const auto shaderTexture{ FindTexture(name) };

        DL_ASSERT(m_Texture2Ds.find(shaderTexture.BindPoint) != m_Texture2Ds.end(),
            "Texture with name [{0}] is not set in the material [{1}]", name, m_Name
        );

        return AsRef<Texture2D>(m_Texture2Ds.at(shaderTexture.BindPoint));
    }

    Ref<TextureCube> D3D11Material::GetTextureCube(const std::string& name) const noexcept
    {
        const auto shaderTexture{ FindTexture(name) };

        DL_ASSERT(m_TextureCubes.find(shaderTexture.BindPoint) != m_TextureCubes.end(),
            "Texture with name [{0}] is not set in the material [{1}]", name, m_Name
        );

        return AsRef<TextureCube>(m_TextureCubes.at(shaderTexture.BindPoint));
    }

    std::size_t D3D11Material::GetHash() const noexcept
    {
        std::size_t hash{ std::hash<Ref<Shader>>{}(m_Shader) };

        for (const auto& [bindPoint, buffer] : m_ConstantBuffers)
        {
            const Buffer& bufferData{ buffer->GetLocalData() };
            hash ^= std::hash<std::string_view>{}(std::string_view{ reinterpret_cast<char*>(bufferData.Data), bufferData.Size });
        }

        for (const auto& [bindPoint, shaderStage] : m_ConstantBuffersShaderStages)
            hash ^= std::hash<uint8_t>{}(shaderStage);

        for (const auto& [bindPoint, texture] : m_Texture2Ds)
            hash ^= std::hash<Ref<Texture2D>>{}(texture);

        for (const auto& [bindPoint, textureCube] : m_TextureCubes)
            hash ^= std::hash<Ref<TextureCube>>{}(textureCube);

        for (const auto& [bindPoint, view] : m_TextureViews)
            hash ^= ByteBufferHash<TextureViewSpecification>{}(view);

        for (const auto& [bindPoint, shaderStage] : m_TextureShaderStages)
            hash ^= std::hash<uint8_t>{}(shaderStage);

        return hash;
    }

    bool D3D11Material::operator==(const Material& other) const noexcept
    {
        if (m_Shader != other.GetShader())
            return false;

        const auto& otherD3D11Material{ static_cast<const D3D11Material&>(other) };

        const bool cbsEqual{
            std::ranges::equal(m_ConstantBuffers, otherD3D11Material.m_ConstantBuffers, [](const auto& a, const auto& b)
            {
                return a.second->GetLocalData() == b.second->GetLocalData();
            })
        };

        if (!cbsEqual)
            return false;

        if (m_ConstantBuffersShaderStages != otherD3D11Material.m_ConstantBuffersShaderStages)
            return false;

        if (m_Texture2Ds != otherD3D11Material.m_Texture2Ds)
            return false;

        if (m_TextureCubes != otherD3D11Material.m_TextureCubes)
            return false;

        if (m_TextureViews != otherD3D11Material.m_TextureViews)
            return false;

        if (m_TextureShaderStages != otherD3D11Material.m_TextureShaderStages)
            return false;

        return true;
    }

    void D3D11Material::SetShaderStageFlags() noexcept
    {
        const auto& shaderReflectionData{ AsRef<D3D11Shader>(m_Shader)->GetReflectionData() };
        
        for (const auto& [name, buffer] : shaderReflectionData.ConstantBuffers)
            m_ConstantBuffersShaderStages[buffer.BindPoint] = buffer.ShaderStageFlags;

        for (const auto& [name, texture] : shaderReflectionData.Textures)
            m_TextureShaderStages[texture.BindPoint] = texture.ShaderStageFlags;
    }

    ShaderBuffer D3D11Material::FindConstantBuffer(const std::string& name) const
    {
        const auto& shaderReflectionData{ AsRef<D3D11Shader>(m_Shader)->GetReflectionData() };

        const auto& it{ shaderReflectionData.ConstantBuffers.find(name) };
        DL_ASSERT(it != shaderReflectionData.ConstantBuffers.end(), "Failed to find constant buffer with name [{0}] in the material [{1}]", name, m_Name);

        return it->second;
    }

    ShaderTexture D3D11Material::FindTexture(const std::string& name) const
    {
        const auto& shaderReflectionData{ AsRef<D3D11Shader>(m_Shader)->GetReflectionData() };

        const auto& it{ shaderReflectionData.Textures.find(name) };
        DL_ASSERT(it != shaderReflectionData.Textures.end(), "Failed to find texture with name [{0}] in the material [{1}]", name, m_Name);

        return it->second;
    }

}