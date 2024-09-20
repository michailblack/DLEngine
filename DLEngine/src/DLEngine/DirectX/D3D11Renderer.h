#pragma once
#include "DLEngine/Renderer/ConstantBuffer.h"
#include "DLEngine/Renderer/RendererAPI.h"
#include "DLEngine/Renderer/Shader.h"
#include "DLEngine/Renderer/StructuredBuffer.h"
#include "DLEngine/Renderer/Texture.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class D3D11Renderer : public RendererAPI
    {
    public:
        void Init() override;
        void Shutdown() override;

        void BeginFrame() override;
        void EndFrame() override;

        Ref<Texture2D> GetBackBufferTexture() override;
        
        void SetConstantBuffers(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<ConstantBuffer>>& constantBuffers) noexcept override;
        void SetTexture2Ds(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<Texture2D>>& textures, const std::vector<TextureViewSpecification>& viewSpecifications) noexcept override;
        void SetTextureCubes(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<TextureCube>>& textures, const std::vector<TextureViewSpecification>& viewSpecifications) noexcept override;
        void SetStructuredBuffers(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<StructuredBuffer>>& structuredBuffers, const std::vector<BufferViewSpecification>& viewSpecifications) noexcept override;
        void SetPrimitiveBuffers(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<PrimitiveBuffer>>& primitiveBuffers, const std::vector<BufferViewSpecification>& viewSpecifications) noexcept override;
        void SetSamplerStates(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<SamplerSpecification>& samplerStates) noexcept override;

        void SetPipeline(const Ref<Pipeline>& pipeline, uint8_t clearAttachmentEnums) noexcept override;
        void SetPipelineCompute(const Ref<PipelineCompute>& pipelineCompute) noexcept override;
        void SetMaterial(const Ref<Material>& material) noexcept override;
        
        void SubmitStaticMeshInstanced(const Ref<Mesh>& mesh, uint32_t submeshIndex, const std::map<uint32_t, Ref<VertexBuffer>>& instanceBuffers, uint32_t instanceCount) noexcept override;
        void SubmitFullscreenQuad() noexcept override;
        void SubmitParticleBillboard(const Ref<VertexBuffer>& particleInstanceBuffer) noexcept override;
        void SubmitParticleBillboardIndirect(const Ref<PrimitiveBuffer>& argumentBuffer, uint32_t argumentOffset) noexcept override;

        void DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) noexcept override;
        void DispatchComputeIndirect(const Ref<PrimitiveBuffer>& argumentBuffer, uint32_t argumentOffset) noexcept override;

        void ClearRenderTargetsState() noexcept override;

        static Microsoft::WRL::ComPtr<ID3D11SamplerState> GetSamplerState(const SamplerSpecification& specification);
        static Microsoft::WRL::ComPtr<ID3D11RasterizerState2> GetRasterizerState(const RasterizerSpecification& specification, bool multisampleEnabled);
        static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> GetDepthStencilState(const DepthStencilSpecification& specification);
        static Microsoft::WRL::ComPtr<ID3D11BlendState1> GetBlendState(const BlendSpecification& specification);

    private:
        static void SetShaderResourceViews(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<ID3D11ShaderResourceView*>& srvs) noexcept;
    };
}