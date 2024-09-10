#pragma once
#include "DLEngine/Renderer/Mesh/Mesh.h"

#include "DLEngine/Renderer/ConstantBuffer.h"
#include "DLEngine/Renderer/Framebuffer.h"
#include "DLEngine/Renderer/Material.h"
#include "DLEngine/Renderer/Pipeline.h"
#include "DLEngine/Renderer/Shader.h"
#include "DLEngine/Renderer/Texture.h"
#include "DLEngine/Renderer/StructuredBuffer.h"

namespace DLEngine
{
    class RendererAPI
    {
    public:
        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual Ref<Texture2D> GetBackBufferTexture() = 0;

        virtual void SetConstantBuffers(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<ConstantBuffer>>& constantBuffers) noexcept = 0;
        virtual void SetTexture2Ds(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<Texture2D>>& textures, const std::vector<TextureViewSpecification>& viewSpecifications) noexcept = 0;
        virtual void SetTextureCubes(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<TextureCube>>& textures, const std::vector<TextureViewSpecification>& viewSpecifications) noexcept = 0;
        virtual void SetStructuredBuffers(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<StructuredBuffer>>& structuredBuffers, const std::vector<BufferViewSpecification>& viewSpecifications) noexcept = 0;
        virtual void SetSamplerStates(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<SamplerSpecification>& samplerStates) noexcept = 0;

        virtual void SetPipeline(const Ref<Pipeline>& pipeline, uint8_t clearAttachmentEnums) noexcept = 0;
        virtual void SetMaterial(const Ref<Material>& material) noexcept = 0;
        virtual void SubmitStaticMeshInstanced(const Ref<Mesh>& mesh, uint32_t submeshIndex, const std::map<uint32_t, Ref<VertexBuffer>>& instanceBuffers, uint32_t instanceCount) noexcept = 0;
        virtual void SubmitFullscreenQuad() noexcept = 0;
        virtual void SubmitParticleBillboard(const Ref<VertexBuffer>& particleInstanceBuffer) noexcept = 0;
    };
}