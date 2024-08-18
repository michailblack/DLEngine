#include "dlpch.h"
#include "SceneRenderer.h"

#include "DLEngine/Renderer/Renderer.h"

namespace DLEngine
{

    SceneRenderer::SceneRenderer(const SceneRendererSpecification& specification)
        : m_ViewportWidth(specification.ViewportWidth)
        , m_ViewportHeight(specification.ViewportHeight)
    {
        m_SceneEnvironment.Skybox = specification.Skybox;

        Init();
    }

    void SceneRenderer::RenderScene(const Ref<Scene>& scene, const PBRSettings& pbrSettings, const PostProcessSettings& postProcessSettings)
    {
        m_Scene = scene;
        m_CBPBRSettings->SetData(Buffer{ &pbrSettings, sizeof(PBRSettings) });
        m_CBPostProcessSettings->SetData(Buffer{ &postProcessSettings, sizeof(PostProcessSettings) });

        PreRender();

        GeometryPass();
        SkyboxPass();
        PostProcessPass();
    }

    void SceneRenderer::Init()
    {
        m_CBCamera = ConstantBuffer::Create(sizeof(CBCamera));
        m_CBPBRSettings = ConstantBuffer::Create(sizeof(PBRSettings));
        m_CBPostProcessSettings = ConstantBuffer::Create(sizeof(PostProcessSettings));

        m_SBDirectionalLights = StructuredBuffer::Create(sizeof(DirectionalLight), 100u);
        m_SBPointLights = StructuredBuffer::Create(sizeof(PointLight), 100u);
        m_SBSpotLights = StructuredBuffer::Create(sizeof(SpotLight), 100u);

        FramebufferSpecification mainFramebufferSpec{};
        mainFramebufferSpec.DebugName = "Main HDR Framebuffer";
        mainFramebufferSpec.Attachments = {
            { TextureFormat::RGBA16F, TextureUsage::TextureAttachment, SamplerSpecification{} },
            { TextureFormat::DEPTH24STENCIL8, TextureUsage::Attachment, SamplerSpecification{} }
        };
        mainFramebufferSpec.Width = m_ViewportWidth;
        mainFramebufferSpec.Height = m_ViewportHeight;
        m_MainFramebuffer = Framebuffer::Create(mainFramebufferSpec);

        PipelineSpecification pbrStaticPipelineSpec{};
        pbrStaticPipelineSpec.DebugName = "PBR_Static Pipeline";
        pbrStaticPipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("PBR_Static");
        pbrStaticPipelineSpec.TargetFramebuffer = m_MainFramebuffer;
        pbrStaticPipelineSpec.DepthStencilState.CompareOp = CompareOperator::Greater;
        pbrStaticPipelineSpec.DepthStencilState.DepthTest = true;
        pbrStaticPipelineSpec.DepthStencilState.DepthWrite = true;
        m_PBRStaticPipeline = Pipeline::Create(pbrStaticPipelineSpec);

        PipelineSpecification emissionPipelineSpec{};
        emissionPipelineSpec.DebugName = "Emission Pipeline";
        emissionPipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("Emission");
        emissionPipelineSpec.TargetFramebuffer = m_MainFramebuffer;
        emissionPipelineSpec.DepthStencilState.CompareOp = CompareOperator::Greater;
        emissionPipelineSpec.DepthStencilState.DepthTest = true;
        emissionPipelineSpec.DepthStencilState.DepthWrite = true;
        m_EmissionPipeline = Pipeline::Create(emissionPipelineSpec);

        PipelineSpecification skyboxPipelineSpec{};
        skyboxPipelineSpec.DebugName = "Skybox Pipeline";
        skyboxPipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("Skybox");
        skyboxPipelineSpec.TargetFramebuffer = m_MainFramebuffer;
        skyboxPipelineSpec.DepthStencilState.CompareOp = CompareOperator::GreaterOrEqual;
        skyboxPipelineSpec.DepthStencilState.DepthTest = true;
        skyboxPipelineSpec.DepthStencilState.DepthWrite = false;
        m_SkyboxPipeline = Pipeline::Create(skyboxPipelineSpec);
        m_SkyboxMaterial = Material::Create(Renderer::GetShaderLibrary()->Get("Skybox"));
        m_SkyboxMaterial->Set("t_Skybox", m_SceneEnvironment.Skybox);

        PipelineSpecification postProcessPipelineSpec{};
        postProcessPipelineSpec.DebugName = "Post Process Pipeline";
        postProcessPipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("PostProcess");
        postProcessPipelineSpec.TargetFramebuffer = Renderer::GetSwapChainTargetFramebuffer();
        postProcessPipelineSpec.DepthStencilState.CompareOp = CompareOperator::Always;
        postProcessPipelineSpec.DepthStencilState.DepthTest = false;
        postProcessPipelineSpec.DepthStencilState.DepthWrite = false;
        m_PostProcessPipeline = Pipeline::Create(postProcessPipelineSpec);

        m_PostProcessMaterial = Material::Create(Renderer::GetShaderLibrary()->Get("PostProcess"));
        m_PostProcessMaterial->Set("PostProcessSettings", m_CBPostProcessSettings);
        m_PostProcessMaterial->Set("t_TextureHDR", AsRef<Texture2D>(m_MainFramebuffer->GetColorAttachment(0u)));

        BuildIrradianceMap();
        BuildPrefilteredMap();
    }

    void SceneRenderer::PreRender()
    {
        const uint32_t directionalLightsCount{ static_cast<uint32_t>(m_Scene->m_LightEnvironment.DirectionalLights.size()) };
        const uint32_t pointLightsCount{ static_cast<uint32_t>(m_Scene->m_LightEnvironment.PointLights.size()) };
        const uint32_t spotLightsCount{ static_cast<uint32_t>(m_Scene->m_LightEnvironment.SpotLights.size()) };
        
        // Binding stuff
        Renderer::SetConstantBuffers(0u, DL_ALL_SHADER_STAGES, { m_CBCamera });
        Renderer::SetConstantBuffers(1u, DL_PIXEL_SHADER_BIT, { m_CBPBRSettings });

        if (directionalLightsCount > 0u)
            Renderer::SetStructuredBuffers(0u, DL_PIXEL_SHADER_BIT, { m_SBDirectionalLights }, { BufferViewSpecification{ 0u, directionalLightsCount } });
        if (pointLightsCount > 0u)
            Renderer::SetStructuredBuffers(1u, DL_PIXEL_SHADER_BIT, { m_SBPointLights }, { BufferViewSpecification{ 0u, pointLightsCount } });
        if (spotLightsCount > 0u)
            Renderer::SetStructuredBuffers(2u, DL_PIXEL_SHADER_BIT, { m_SBSpotLights }, { BufferViewSpecification{ 0u, spotLightsCount } });

        Renderer::SetTextureCubes(8u, DL_PIXEL_SHADER_BIT,
            { m_SceneEnvironment.IrradianceMap, m_SceneEnvironment.PrefilteredMap },
            { TextureViewSpecification{}, TextureViewSpecification{} }
        );
        Renderer::SetTexture2Ds(10u, DL_PIXEL_SHADER_BIT, { Renderer::GetBRDFLUT() }, { TextureViewSpecification{} });

        // Setting/updating stuff
        m_Scene->m_MeshRegistry.UpdateInstanceBuffers();
        
        // Updating light buffers
        {
            // Directional lights
            if (m_SBDirectionalLights->GetElementsCount() < directionalLightsCount)
                m_SBDirectionalLights = StructuredBuffer::Create(sizeof(DirectionalLight), directionalLightsCount);

            auto directionalLightsData{ m_SBDirectionalLights->Map().As<DirectionalLight>() };
            for (uint32_t i{ 0u }; i < directionalLightsCount; ++i)
                directionalLightsData[i] = m_Scene->m_LightEnvironment.DirectionalLights[i];
            m_SBDirectionalLights->Unmap();

            // Point lights
            if (m_SBPointLights->GetElementsCount() < pointLightsCount)
                m_SBPointLights = StructuredBuffer::Create(sizeof(PointLight), pointLightsCount);

            auto pointLightsData{ m_SBPointLights->Map().As<PointLight>() };
            for (uint32_t i{ 0u }; i < pointLightsCount; ++i)
            {
                auto& [light, instance]{ m_Scene->m_LightEnvironment.PointLights[i] };

                const auto& transform{ instance->Get<Math::Mat4x4>("TRANSFORM") };

                PointLight transformedLight{ light };
                transformedLight.Position = Math::PointToSpace(light.Position, transform);

                pointLightsData[i] = transformedLight;
            }
            m_SBPointLights->Unmap();

            // Spot lights
            if (m_SBSpotLights->GetElementsCount() < spotLightsCount)
                m_SBSpotLights = StructuredBuffer::Create(sizeof(SpotLight), spotLightsCount);

            auto spotLightsData{ m_SBSpotLights->Map().As<SpotLight>() };
            for (uint32_t i{ 0u }; i < spotLightsCount; ++i)
            {
                auto& [light, instance]{ m_Scene->m_LightEnvironment.SpotLights[i] };

                const auto& transform{ instance->Get<Math::Mat4x4>("TRANSFORM") };

                SpotLight transformedLight{ light };
                transformedLight.Position = Math::PointToSpace(light.Position, transform);
                transformedLight.Direction = Math::Normalize(Math::DirectionToSpace(light.Direction, transform));

                spotLightsData[i] = transformedLight;
            }
            m_SBSpotLights->Unmap();
        }

        m_MainFramebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
    }

    void SceneRenderer::GeometryPass()
    {
        const auto& sceneCamera{ m_Scene->m_SceneCameraController.GetCamera() };
        UpdateCBCamera(sceneCamera);

        Renderer::SetPipeline(m_PBRStaticPipeline, true);
        SubmitAllMeshesForShader("PBR_Static");

        Renderer::SetPipeline(m_EmissionPipeline, false);
        SubmitAllMeshesForShader("Emission");
    }

    void SceneRenderer::SkyboxPass()
    {
        Renderer::SetPipeline(m_SkyboxPipeline, false);
        
        Renderer::SetMaterial(m_SkyboxMaterial);
        Renderer::SubmitFullscreenQuad();
    }

    void SceneRenderer::PostProcessPass()
    {
        Renderer::SetPipeline(m_PostProcessPipeline, true);
        Renderer::SetMaterial(m_PostProcessMaterial);
        Renderer::SubmitFullscreenQuad();
    }

    void SceneRenderer::UpdateCBCamera(const Camera& camera)
    {
        CBCamera cameraData{};
        cameraData.Projection = camera.GetProjectionMatrix();
        cameraData.InvProjection = Math::Mat4x4::Inverse(cameraData.Projection);
        cameraData.View = camera.GetViewMatrix();
        cameraData.InvView = Math::Mat4x4::Inverse(cameraData.View);
        cameraData.ViewProjection = cameraData.View * cameraData.Projection;
        cameraData.InvViewProjection = Math::Mat4x4::Inverse(cameraData.ViewProjection);
        cameraData.CameraPosition = camera.GetPosition();
        cameraData.BL = camera.ConstructFrustumPosRotOnly(Math::Vec2{ 0.0f, static_cast<float>(m_ViewportHeight) });
        cameraData.BL2BR = camera.ConstructFrustumPosRotOnly(Math::Vec2{ static_cast<float>(m_ViewportWidth), static_cast<float>(m_ViewportHeight) }) - cameraData.BL;
        cameraData.BL2TL = camera.ConstructFrustumPosRotOnly(Math::Vec2{ 0.0f, 0.0f }) - cameraData.BL;

        m_CBCamera->SetData(Buffer{ &cameraData, sizeof(CBCamera) });
    }

    void SceneRenderer::SubmitAllMeshesForShader(const std::string_view shaderName)
    {
        const auto& meshBatch{ m_Scene->m_MeshRegistry.GetMeshBatch(shaderName) };
        for (const auto& [mesh, submeshBatch] : meshBatch.SubmeshBatches)
        {
            for (uint32_t submeshIndex{ 0u }; submeshIndex < mesh->GetSubmeshes().size(); ++submeshIndex)
            {
                for (const auto& [material, instanceBatch] : submeshBatch.MaterialBatches[submeshIndex].InstanceBatches)
                {
                    const uint32_t instanceCount{ static_cast<uint32_t>(instanceBatch.SubmeshInstances.size()) };
                    Renderer::SetMaterial(material);
                    Renderer::SubmitStaticMeshInstanced(mesh, submeshIndex, instanceBatch.InstanceBuffer, instanceCount);
                }
            }
        }
    }

    void SceneRenderer::BuildIrradianceMap()
    {
        const uint32_t brdfLUTSize{ Renderer::GetBRDFLUT()->GetWidth() };

        TextureSpecification irradianceMapSpec{};
        irradianceMapSpec.DebugName = "Irradiance Map";
        irradianceMapSpec.Format = TextureFormat::RGBA16F;
        irradianceMapSpec.Usage = TextureUsage::TextureAttachment;
        irradianceMapSpec.Width = brdfLUTSize;
        irradianceMapSpec.Height = brdfLUTSize;
        irradianceMapSpec.Layers = 6u;
        m_SceneEnvironment.IrradianceMap = TextureCube::Create(irradianceMapSpec);

        FramebufferSpecification irradianceMapFBSpec{};
        irradianceMapFBSpec.DebugName = "Irradiance Map Framebuffer";
        irradianceMapFBSpec.ExistingAttachments[0] = m_SceneEnvironment.IrradianceMap;
        irradianceMapFBSpec.ClearColor = Math::Vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
        irradianceMapFBSpec.AttachmentsType = TextureType::TextureCube;
        irradianceMapFBSpec.Width = brdfLUTSize;
        irradianceMapFBSpec.Height = brdfLUTSize;
        const Ref<Framebuffer> irradianceMapFB{ Framebuffer::Create(irradianceMapFBSpec) };

        PipelineSpecification irradianceMapPipelineSpec{};
        irradianceMapPipelineSpec.DebugName = "Irradiance Map Pipeline";
        irradianceMapPipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("EnvironmentIrradiance");
        irradianceMapPipelineSpec.TargetFramebuffer = irradianceMapFB;
        irradianceMapPipelineSpec.DepthStencilState.CompareOp = CompareOperator::Always;
        irradianceMapPipelineSpec.DepthStencilState.DepthTest = false;
        irradianceMapPipelineSpec.DepthStencilState.DepthWrite = false;
        const Ref<Pipeline> irradianceMapPipeline{ Pipeline::Create(irradianceMapPipelineSpec) };

        struct
        {
            uint32_t EnvMapSize;
            uint32_t SamplesCount;
            float _padding[2];
        } CBEnvironmentMapping;
        CBEnvironmentMapping.EnvMapSize = brdfLUTSize;
        CBEnvironmentMapping.SamplesCount = brdfLUTSize;

        Ref<ConstantBuffer> cbEnvironmentMapping{ ConstantBuffer::Create(sizeof(CBEnvironmentMapping)) };
        cbEnvironmentMapping->SetData(Buffer{ &CBEnvironmentMapping, sizeof(CBEnvironmentMapping) });

        Renderer::SetPipeline(irradianceMapPipeline, true);
        Renderer::SetConstantBuffers(0u, DL_PIXEL_SHADER_BIT, { cbEnvironmentMapping });
        Renderer::SetSamplerStates(0u, DL_PIXEL_SHADER_BIT, { SamplerSpecification{ SamplerSpecification{ TextureAddress::Clamp, TextureFilter::Anisotropic8, CompareOperator::Never } } });
        Renderer::SetTextureCubes(0u, DL_PIXEL_SHADER_BIT, { m_SceneEnvironment.Skybox }, { TextureViewSpecification{} });
        Renderer::SubmitFullscreenQuad();
    }

    void SceneRenderer::BuildPrefilteredMap()
    {
        const uint32_t brdfLUTSize{ Renderer::GetBRDFLUT()->GetWidth() };
        
        TextureSpecification prefilteredMapSpec{};
        prefilteredMapSpec.DebugName = "Prefiltered Map";
        prefilteredMapSpec.Format = TextureFormat::RGBA16F;
        prefilteredMapSpec.Usage = TextureUsage::TextureAttachment;
        prefilteredMapSpec.Width = brdfLUTSize;
        prefilteredMapSpec.Height = brdfLUTSize;
        prefilteredMapSpec.Mips = static_cast<uint32_t>(Math::Log2(static_cast<float>(brdfLUTSize))) + 1u;
        prefilteredMapSpec.Layers = 6u;
        m_SceneEnvironment.PrefilteredMap = TextureCube::Create(prefilteredMapSpec);

        FramebufferSpecification prefilteredMapFBSpec{};
        prefilteredMapFBSpec.DebugName = "Prefiltered Map Framebuffer";
        prefilteredMapFBSpec.ExistingAttachments[0] = m_SceneEnvironment.PrefilteredMap;
        prefilteredMapFBSpec.ClearColor = Math::Vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
        prefilteredMapFBSpec.AttachmentsType = TextureType::TextureCube;
        std::vector<Ref<Framebuffer>> prefilteredMapMipFBs{ prefilteredMapSpec.Mips };
        for (uint32_t mip{ 0u }; mip < prefilteredMapSpec.Mips; ++mip)
        {
            prefilteredMapFBSpec.Width = brdfLUTSize >> mip;
            prefilteredMapFBSpec.Height = brdfLUTSize >> mip;
            prefilteredMapMipFBs[mip] = Framebuffer::Create(prefilteredMapFBSpec);

            TextureViewSpecification framebufferViewSpec{};
            framebufferViewSpec.Subresource.BaseMip = mip;
            framebufferViewSpec.Subresource.MipsCount = 1u; // Actually ignored in D3D11TextureCube
            framebufferViewSpec.Subresource.BaseLayer = 0u;
            framebufferViewSpec.Subresource.LayersCount = 6u;
            prefilteredMapMipFBs[mip]->SetColorAttachmentViewSpecification(0u, framebufferViewSpec);
        }

        PipelineSpecification prefilteredMapPipelineSpec{};
        prefilteredMapPipelineSpec.DebugName = "Prefiltered Map Pipeline";
        prefilteredMapPipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("PrefilteredEnvironment");
        prefilteredMapPipelineSpec.DepthStencilState.CompareOp = CompareOperator::Always;
        prefilteredMapPipelineSpec.DepthStencilState.DepthTest = false;
        prefilteredMapPipelineSpec.DepthStencilState.DepthWrite = false;
        std::vector<Ref<Pipeline>> prefilteredMapMipPipelines{ prefilteredMapSpec.Mips };
        for (uint32_t i{ 0u }; i < prefilteredMapSpec.Mips; ++i)
        {
            prefilteredMapPipelineSpec.TargetFramebuffer = prefilteredMapMipFBs[i];
            prefilteredMapMipPipelines[i] = Pipeline::Create(prefilteredMapPipelineSpec);
        }

        struct
        {
            uint32_t EnvMapSize;
            uint32_t SamplesCount;
            float Roughness;
            float _padding;
        } CBEnvironmentMapping;
        CBEnvironmentMapping.EnvMapSize = brdfLUTSize;
        CBEnvironmentMapping.SamplesCount = brdfLUTSize;
        Ref<ConstantBuffer> cbEnvironmentMapping{ ConstantBuffer::Create(sizeof(CBEnvironmentMapping)) };

        Renderer::SetConstantBuffers(0u, DL_PIXEL_SHADER_BIT, { cbEnvironmentMapping });
        Renderer::SetSamplerStates(0u, DL_PIXEL_SHADER_BIT, { SamplerSpecification{ SamplerSpecification{ TextureAddress::Clamp, TextureFilter::Anisotropic8, CompareOperator::Never } } });
        Renderer::SetTextureCubes(0u, DL_PIXEL_SHADER_BIT, { m_SceneEnvironment.Skybox }, { TextureViewSpecification{} });

        const float roughnessStep{ 1.0f / static_cast<float>(prefilteredMapSpec.Mips - 1u) };
        for (uint32_t mip{ 0u }; mip < prefilteredMapSpec.Mips; ++mip)
        {
            CBEnvironmentMapping.Roughness = static_cast<float>(mip) * roughnessStep;
            cbEnvironmentMapping->SetData(Buffer{ &CBEnvironmentMapping, sizeof(CBEnvironmentMapping) });

            Renderer::SetPipeline(prefilteredMapMipPipelines[mip], true);
            Renderer::SubmitFullscreenQuad();
        }
    }

}