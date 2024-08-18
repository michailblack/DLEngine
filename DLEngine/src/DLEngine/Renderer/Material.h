#pragma once
#include "DLEngine/Renderer/ConstantBuffer.h"
#include "DLEngine/Renderer/Shader.h"
#include "DLEngine/Renderer/Texture.h"

namespace DLEngine
{
    class Material
    {
    public:
        virtual ~Material() = default;

        virtual void Set(const std::string& name, const Ref<ConstantBuffer>& buffer) noexcept = 0;
        virtual void Set(const std::string& name, const Ref<Texture2D>& texture) noexcept = 0;
        virtual void Set(const std::string& name, const Ref<TextureCube>& texture) noexcept = 0;

        virtual void SetTextureView(const std::string& name, const TextureViewSpecification& view) noexcept = 0;

        virtual Ref<ConstantBuffer> GetConstantBuffer(const std::string& name) const noexcept = 0;
        virtual Ref<Texture2D> GetTexture2D(const std::string& name) const noexcept = 0;
        virtual Ref<TextureCube> GetTextureCube(const std::string& name) const noexcept = 0;

        virtual const std::map<uint32_t, Ref<ConstantBuffer>>& GetConstantBuffers() const noexcept = 0;
        virtual const std::unordered_map<uint32_t, uint8_t>& GetConstantBuffersShaderStages() const noexcept = 0;

        virtual const std::map<uint32_t, Ref<Texture2D>>& GetTexture2Ds() const noexcept = 0;
        virtual const std::map<uint32_t, Ref<TextureCube>>& GetTextureCubes() const noexcept = 0;
        virtual const std::unordered_map<uint32_t, TextureViewSpecification>& GetTextureViews() const noexcept = 0;
        virtual const std::unordered_map<uint32_t, uint8_t>& GetTextureShaderStages() const noexcept = 0;

        virtual Ref<Shader> GetShader() const noexcept = 0;

        virtual const std::string& GetName() const noexcept = 0;

        static Ref<Material> Create(const Ref<Shader>& shader, const std::string& name = "");
        static Ref<Material> Copy(const Ref<Material>& material, const std::string& name = "");
    };
}