#include "dlpch.h"
#include "Renderer.h"

#include "DLEngine/DirectX/D3D11Renderer.h"

#include "DLEngine/Renderer/RendererAPI.h"

namespace DLEngine
{
    namespace
    {
        RendererAPI* s_RendererAPI{ nullptr };

        RendererAPI* InitRendererAPI()
        {
            return new D3D11Renderer;
        }

        constexpr uint32_t s_BRDFLUTSize{ 2048u };

        struct RendererData
        {
            Ref<MeshLibrary> MeshLib;
            Ref<ShaderLibrary> ShaderLib;
            Ref<TextureLibrary> TextureLib;

            Ref<Framebuffer> SwapChainFB;

            Ref<Texture2D> BRDFLUT;
        };

        RendererData* s_RendererData{ nullptr };
    }

    void Renderer::Init()
    {
        s_RendererAPI = InitRendererAPI();
        s_RendererAPI->Init();

        s_RendererData = new RendererData;

        s_RendererData->MeshLib = CreateRef<MeshLibrary>();
        s_RendererData->MeshLib->Init();

        s_RendererData->ShaderLib = CreateRef<ShaderLibrary>();
        s_RendererData->ShaderLib->Init();

        s_RendererData->TextureLib = CreateRef<TextureLibrary>();

        FramebufferSpecification framebufferSpec{};
        framebufferSpec.DebugName = "Swap chain target";
        framebufferSpec.SwapChainTarget = true;
        s_RendererData->SwapChainFB = Framebuffer::Create(framebufferSpec);

        InitBRDFLUT();

        DL_LOG_INFO_TAG("Renderer", "Renderer Initialized");
    }

    void Renderer::Shutdown()
    {
        delete s_RendererData;
        s_RendererAPI->Shutdown();
    }

    void Renderer::BeginFrame()
    {
        s_RendererAPI->BeginFrame();
    }

    void Renderer::EndFrame()
    {
        s_RendererAPI->EndFrame();
    }

    Ref<MeshLibrary> Renderer::GetMeshLibrary() noexcept
    {
        return s_RendererData->MeshLib;
    }

    Ref<ShaderLibrary> Renderer::GetShaderLibrary() noexcept
    {
        return s_RendererData->ShaderLib;
    }

    Ref<TextureLibrary> Renderer::GetTextureLibrary() noexcept
    {
        return s_RendererData->TextureLib;
    }

    Ref<Texture2D> Renderer::GetBRDFLUT() noexcept
    {
        return s_RendererData->BRDFLUT;
    }

    Ref<Texture2D> Renderer::GetBackBufferTexture()
    {
        return s_RendererAPI->GetBackBufferTexture();
    }

    Ref<Framebuffer> Renderer::GetSwapChainTargetFramebuffer() noexcept
    {
        return s_RendererData->SwapChainFB;
    }

    void Renderer::RecreateSwapChainTargetFramebuffer()
    {
        s_RendererData->SwapChainFB->Resize(0u, 0u, true);
    }

    void Renderer::InvalidateSwapChainTargetFramebuffer() noexcept
    {
        s_RendererData->SwapChainFB->Invalidate();
    }

    void Renderer::SetConstantBuffers(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<ConstantBuffer>>& constantBuffers) noexcept
    {
        s_RendererAPI->SetConstantBuffers(startSlot, shaderStageFlags, constantBuffers);
    }

    void Renderer::SetTexture2Ds(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<Texture2D>>& textures, const std::vector<TextureViewSpecification>& viewSpecifications) noexcept
    {
        s_RendererAPI->SetTexture2Ds(startSlot, shaderStageFlags, textures, viewSpecifications);
    }

    void Renderer::SetTextureCubes(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<TextureCube>>& textures, const std::vector<TextureViewSpecification>& viewSpecifications) noexcept
    {
        s_RendererAPI->SetTextureCubes(startSlot, shaderStageFlags, textures, viewSpecifications);
    }

    void Renderer::SetStructuredBuffers(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<StructuredBuffer>>& structuredBuffers, const std::vector<BufferViewSpecification>& viewSpecifications) noexcept
    {
        s_RendererAPI->SetStructuredBuffers(startSlot, shaderStageFlags, structuredBuffers, viewSpecifications);
    }

    void Renderer::SetSamplerStates(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<SamplerSpecification>& samplerStates) noexcept
    {
        s_RendererAPI->SetSamplerStates(startSlot, shaderStageFlags, samplerStates);
    }

    void Renderer::SetPipeline(const Ref<Pipeline>& pipeline, uint8_t clearAttachmentEnums) noexcept
    {
        s_RendererAPI->SetPipeline(pipeline, clearAttachmentEnums);
    }

    void Renderer::SetMaterial(const Ref<Material>& material) noexcept
    {
        s_RendererAPI->SetMaterial(material);
    }

    void Renderer::SubmitStaticMeshInstanced(const Ref<Mesh>& mesh, uint32_t submeshIndex, const std::map<uint32_t, Ref<VertexBuffer>>& instanceBuffers, uint32_t instanceCount) noexcept
    {
        s_RendererAPI->SubmitStaticMeshInstanced(mesh, submeshIndex, instanceBuffers, instanceCount);
    }

    void Renderer::SubmitFullscreenQuad() noexcept
    {
        s_RendererAPI->SubmitFullscreenQuad();
    }

    void Renderer::SubmitParticleBillboard(const Ref<VertexBuffer>& particleInstanceBuffer) noexcept
    {
        s_RendererAPI->SubmitParticleBillboard(particleInstanceBuffer);
    }

    void Renderer::InitBRDFLUT()
    {
        TextureSpecification brdfLUTSpec{};
        brdfLUTSpec.DebugName = "BRDF LUT";
        brdfLUTSpec.Format = TextureFormat::RG16_FLOAT;
        brdfLUTSpec.Usage = TextureUsage::TextureAttachment;
        brdfLUTSpec.Width = s_BRDFLUTSize;
        brdfLUTSpec.Height = s_BRDFLUTSize;
        s_RendererData->BRDFLUT = Texture2D::Create(brdfLUTSpec);

        ShaderSpecification brdfLUTShaderSpec{};
        brdfLUTShaderSpec.Path = Shader::GetShaderDirectoryPath() / "BRDFLUT.hlsl";
        brdfLUTShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        brdfLUTShaderSpec.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        const Ref<Shader> brdfLUTShader{ Shader::Create(brdfLUTShaderSpec) };

        FramebufferSpecification brdfLUTFBSpec{};
        brdfLUTFBSpec.DebugName = "BRDF LUT Framebuffer";
        brdfLUTFBSpec.ExistingAttachments[0] = s_RendererData->BRDFLUT;
        brdfLUTFBSpec.ClearColor = Math::Vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
        brdfLUTFBSpec.Width = s_BRDFLUTSize;
        brdfLUTFBSpec.Height = s_BRDFLUTSize;
        const Ref<Framebuffer> brdfLUTFB{ Framebuffer::Create(brdfLUTFBSpec) };

        PipelineSpecification brdfLUTPipelineSpec{};
        brdfLUTPipelineSpec.DebugName = "BRDF LUT Pipeline";
        brdfLUTPipelineSpec.Shader = brdfLUTShader;
        brdfLUTPipelineSpec.TargetFramebuffer = brdfLUTFB;
        brdfLUTPipelineSpec.DepthStencilState.DepthCompareOp = CompareOperator::Always;
        brdfLUTPipelineSpec.DepthStencilState.DepthTest = false;
        brdfLUTPipelineSpec.DepthStencilState.DepthWrite = false;
        const Ref<Pipeline> brdfLUTPipeline{ Pipeline::Create(brdfLUTPipelineSpec) };

        struct
        {
            uint32_t EnvMapSize{ s_BRDFLUTSize };
            uint32_t SamplesCount{ s_BRDFLUTSize };
            float _padding[2];
        } CBEnvironmentMapping;

        Ref<ConstantBuffer> cbEnvironmentMapping{ ConstantBuffer::Create(sizeof(CBEnvironmentMapping)) };
        cbEnvironmentMapping->SetData(Buffer{ &CBEnvironmentMapping, sizeof(CBEnvironmentMapping) });

        SetPipeline(brdfLUTPipeline, DL_CLEAR_COLOR_ATTACHMENT);
        SetConstantBuffers(0u, ShaderStage::DL_PIXEL_SHADER_BIT, { cbEnvironmentMapping });
        SubmitFullscreenQuad();
    }

}
