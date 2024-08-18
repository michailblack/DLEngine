#pragma once
#include "DLEngine/Renderer/Material.h"

namespace DLEngine
{
    class D3D11Material : public Material
    {
    public:
        D3D11Material(const Ref<Shader>& shader, const std::string& name) noexcept;
        D3D11Material(const Ref<Material>& material, const std::string& name) noexcept;

        void Set(const std::string& name, const Ref<ConstantBuffer>& buffer) noexcept override;
        void Set(const std::string& name, const Ref<Texture2D>& texture) noexcept override;
        void Set(const std::string& name, const Ref<TextureCube>& texture) noexcept override;

        void SetTextureView(const std::string& name, const TextureViewSpecification& view) noexcept override;

        Ref<ConstantBuffer> GetConstantBuffer(const std::string& name) const noexcept override;
        Ref<Texture2D> GetTexture2D(const std::string& name) const noexcept override;
        Ref<TextureCube> GetTextureCube(const std::string& name) const noexcept override;

        const std::map<uint32_t, Ref<ConstantBuffer>>& GetConstantBuffers() const noexcept override { return m_ConstantBuffers; }
        const std::unordered_map<uint32_t, uint8_t>& GetConstantBuffersShaderStages() const noexcept override { return m_ConstantBuffersShaderStages; }
        const std::map<uint32_t, Ref<Texture2D>>& GetTexture2Ds() const noexcept override { return m_Texture2Ds; }
        const std::map<uint32_t, Ref<TextureCube>>& GetTextureCubes() const noexcept override { return m_TextureCubes; }
        const std::unordered_map<uint32_t, TextureViewSpecification>& GetTextureViews() const noexcept override { return m_TextureViews; }
        const std::unordered_map<uint32_t, uint8_t>& GetTextureShaderStages() const noexcept override { return m_TextureShaderStages; }

        Ref<Shader> GetShader() const noexcept override { return m_Shader; }

        const std::string& GetName() const noexcept override { return m_Name; }

    private:
        void SetShaderStageFlags() noexcept;

        ShaderBuffer FindConstantBuffer(const std::string& name) const;
        ShaderTexture FindTexture(const std::string& name) const;

    private:
        std::string m_Name;

        Ref<Shader> m_Shader;

        std::map<uint32_t, Ref<ConstantBuffer>> m_ConstantBuffers{};
        std::unordered_map<uint32_t, uint8_t> m_ConstantBuffersShaderStages{};

        std::map<uint32_t, Ref<Texture2D>> m_Texture2Ds{};
        std::map<uint32_t, Ref<TextureCube>> m_TextureCubes{};
        std::unordered_map<uint32_t, TextureViewSpecification> m_TextureViews{};
        std::unordered_map<uint32_t, uint8_t> m_TextureShaderStages{};
    };
}