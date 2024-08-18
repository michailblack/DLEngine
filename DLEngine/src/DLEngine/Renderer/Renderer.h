#pragma once
#include "DLEngine/Renderer/Mesh/Mesh.h"

#include "DLEngine/Renderer/ConstantBuffer.h"
#include "DLEngine/Renderer/StructuredBuffer.h"
#include "DLEngine/Renderer/Framebuffer.h"
#include "DLEngine/Renderer/Material.h"
#include "DLEngine/Renderer/Pipeline.h"
#include "DLEngine/Renderer/Shader.h"
#include "DLEngine/Renderer/Texture.h"

namespace DLEngine
{
    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();

        static Ref<MeshLibrary> GetMeshLibrary() noexcept;
        static Ref<ShaderLibrary> GetShaderLibrary() noexcept;
        static Ref<TextureLibrary> GetTextureLibrary() noexcept;

        static Ref<Texture2D> GetBRDFLUT() noexcept;

        static Ref<Texture2D> GetBackBufferTexture();
        static Ref<Framebuffer> GetSwapChainTargetFramebuffer() noexcept;
        static void RecreateSwapChainTargetFramebuffer();
        static void InvalidateSwapChainTargetFramebuffer() noexcept;

        static void SetConstantBuffers(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<ConstantBuffer>>& constantBuffers) noexcept;
        static void SetTexture2Ds(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<Texture2D>>& textures, const std::vector<TextureViewSpecification>& viewSpecifications) noexcept;
        static void SetTextureCubes(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<TextureCube>>& textures, const std::vector<TextureViewSpecification>& viewSpecifications) noexcept;
        static void SetStructuredBuffers(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<StructuredBuffer>>& structuredBuffers, const std::vector<BufferViewSpecification>& viewSpecifications) noexcept;
        static void SetSamplerStates(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<SamplerSpecification>& samplerStates) noexcept;

        static void SetPipeline(const Ref<Pipeline>& pipeline, bool clearAttachments) noexcept;
        static void SetMaterial(const Ref<Material>& material) noexcept;

        static void SubmitStaticMeshInstanced(const Ref<Mesh>& mesh, uint32_t submeshIndex, const Ref<VertexBuffer>& instanceBuffer, uint32_t instanceCount) noexcept;
        static void SubmitFullscreenQuad() noexcept;

    private:
        static void InitBRDFLUT();
    };
}
