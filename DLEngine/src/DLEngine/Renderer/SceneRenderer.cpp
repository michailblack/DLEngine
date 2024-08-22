#include "dlpch.h"
#include "SceneRenderer.h"

#include "DLEngine/Math/Intersections.h"

#include "DLEngine/Renderer/Renderer.h"

namespace DLEngine
{
    namespace
    {
        enum BindingPoint : uint32_t
        {
            // Constant buffers
            BP_CB_CAMERA              = 0u,
            BP_CB_PBR_SETTINGS        = 1u,
            BP_CB_SHADOW_MAPPING_DATA = 2u,
            BP_CB_LIGHTS_COUNT        = 3u,

            BP_CB_NEXT_FREE,
            
            // Structured buffers
            BP_SB_DIRECTIONAL_LIGHTS      = 0u,
            BP_SB_POINT_LIGHTS            = 1u,
            BP_SB_SPOT_LIGHTS             = 2u,
            BP_SB_DIRECTIONAL_LIGHTS_POVS = 3u,
            BP_SB_POINT_LIGHTS_POVS       = 4u,
            BP_SB_SPOT_LIGHTS_POVS        = 5u,

            BP_SB_NEXT_FREE,

            // Textures
            // Shadow maps
            BP_TEX_DIRECTIONAL_SHADOW_MAPS = 6u,
            BP_TEX_POINT_SHADOW_MAPS       = 7u,
            BP_TEX_SPOT_SHADOW_MAPS        = 8u,
            
            // PBR textures
            BP_TEX_ALBEDO_MAP    = 9u,
            BP_TEX_NORMAL_MAP    = 10u,
            BP_TEX_METALNESS_MAP = 11u,
            BP_TEX_ROUGHNESS_MAP = 12u,

            // Environment maps
            BP_TEX_IRRADIANCE_MAP  = 13u,
            BP_TEX_PREFILTERED_MAP = 14u,
            BP_TEX_BRDF_LUT        = 15u,

            BP_TEX_NEXT_FREE,
        };

        struct CBDirectionalLightShadowMapData
        {
            Math::Vec3 LightDirection;
            float WorldTexelSize;
        };

        struct CBOmnidirectionalLightShadowData
        {
            std::array<Math::Mat4x4, 6u> LightViewProjections;
            Math::Vec3 LightPosition;
            float WorldTexelSize;
        };

        struct CBSpotLightShadowMapData
        {
            Math::Vec3 LightPosition;
            float WorldTexelSize;
        };

        struct CBShadowMappingData
        {
            uint32_t ShadowMapSize;
            uint32_t UseDirectionalShadows{ static_cast<uint32_t>(true) };
            uint32_t UseOmnidirectionalShadows{ static_cast<uint32_t>(true) };
            uint32_t UseSpotShadows{ static_cast<uint32_t>(true) };
            uint32_t UsePCF{ static_cast<uint32_t>(true) };
            float _padding[3];
        };
    }

    SceneRenderer::SceneRenderer(const SceneRendererSpecification& specification)
        : m_ViewportWidth(specification.ViewportWidth)
        , m_ViewportHeight(specification.ViewportHeight)
    {
        m_SceneEnvironment.Skybox = specification.Skybox;

        Init();
    }

    void SceneRenderer::RenderScene(const Ref<Scene>& scene)
    {
        m_Scene = scene;

        PreRender();

        ShadowPass();
        GeometryPass();
        SkyboxPass();
        PostProcessPass();
    }

    void SceneRenderer::SetShadowMapSettings(const ShadowMapSettings& shadowMapSettings)
    {
        m_SceneShadowEnvironment.ForceRecreateDirectionalLightShadowMaps = m_SceneShadowEnvironment.Settings.MapSize != shadowMapSettings.MapSize ||
            m_SceneShadowEnvironment.Settings.DirectionalLightShadowMargin != shadowMapSettings.DirectionalLightShadowMargin ||
            m_SceneShadowEnvironment.Settings.DirectionalLightShadowDistance != shadowMapSettings.DirectionalLightShadowDistance;

        m_SceneShadowEnvironment.Settings = shadowMapSettings;

        CBShadowMappingData shadowMappingData{};
        shadowMappingData.ShadowMapSize = shadowMapSettings.MapSize;
        shadowMappingData.UseDirectionalShadows = static_cast<uint32_t>(shadowMapSettings.UseDirectionalShadows);
        shadowMappingData.UseOmnidirectionalShadows = static_cast<uint32_t>(shadowMapSettings.UseOmnidirectionalShadows);
        shadowMappingData.UseSpotShadows = static_cast<uint32_t>(shadowMapSettings.UseSpotShadows);
        shadowMappingData.UsePCF = static_cast<uint32_t>(shadowMapSettings.UsePCF);
        m_CBShadowMappingData->SetData(Buffer{ &shadowMappingData, sizeof(CBShadowMappingData) });
    }

    void SceneRenderer::Init()
    {
        m_CBCamera = ConstantBuffer::Create(sizeof(CBCamera));
        m_CBPBRSettings = ConstantBuffer::Create(sizeof(CBPBRSettings));
        m_CBLightsCount = ConstantBuffer::Create(sizeof(CBLightsCount));
        m_CBPostProcessSettings = ConstantBuffer::Create(sizeof(CBPostProcessSettings));
        m_SceneShadowEnvironment.CBDirectionalLightData = ConstantBuffer::Create(sizeof(CBDirectionalLightShadowMapData));
        m_SceneShadowEnvironment.CBPointLightData = ConstantBuffer::Create(sizeof(CBOmnidirectionalLightShadowData));
        m_SceneShadowEnvironment.CBSpotLightData = ConstantBuffer::Create(sizeof(CBSpotLightShadowMapData));
        m_CBShadowMappingData = ConstantBuffer::Create(sizeof(CBShadowMappingData));

        m_SBDirectionalLights = StructuredBuffer::Create(sizeof(DirectionalLight), 100u);
        m_SBPointLights = StructuredBuffer::Create(sizeof(PointLight), 100u);
        m_SBSpotLights = StructuredBuffer::Create(sizeof(SpotLight), 100u);
        m_SceneShadowEnvironment.SBDirectionalLightsPOVs = StructuredBuffer::Create(sizeof(Math::Mat4x4), 100u);
        m_SceneShadowEnvironment.SBPointLightsPOVs = StructuredBuffer::Create(sizeof(Math::Mat4x4), 100u);
        m_SceneShadowEnvironment.SBSpotLightsPOVs = StructuredBuffer::Create(sizeof(Math::Mat4x4), 100u);

        FramebufferSpecification mainFramebufferSpec{};
        mainFramebufferSpec.DebugName = "Main HDR Framebuffer";
        mainFramebufferSpec.Attachments = {
            { TextureFormat::RGBA16F, TextureUsage::TextureAttachment, 1u, 1u },
            { TextureFormat::DEPTH24STENCIL8, TextureUsage::Attachment, 1u, 1u }
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

        FramebufferSpecification directionalShadowMapFBSpec{};
        directionalShadowMapFBSpec.DebugName = "Directional Shadow Map Framebuffer";
        directionalShadowMapFBSpec.Attachments = { { TextureFormat::DEPTH_R24G8T, TextureUsage::TextureAttachment, 1u, 1u } };
        directionalShadowMapFBSpec.Width = m_SceneShadowEnvironment.Settings.MapSize;
        directionalShadowMapFBSpec.Height = m_SceneShadowEnvironment.Settings.MapSize;
        m_DirectionalShadowMapFramebuffer = Framebuffer::Create(directionalShadowMapFBSpec);

        m_SceneShadowEnvironment.DirectionalShadowMaps = AsRef<Texture2D>(m_DirectionalShadowMapFramebuffer->GetDepthAttachment());

        PipelineSpecification directionalShadowMapPipelineSpec{};
        directionalShadowMapPipelineSpec.DebugName = "Directional Shadow Map Pipeline";
        directionalShadowMapPipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("DirectionalLightShadowMap");
        directionalShadowMapPipelineSpec.TargetFramebuffer = m_DirectionalShadowMapFramebuffer;
        directionalShadowMapPipelineSpec.DepthStencilState.CompareOp = CompareOperator::Greater;
        directionalShadowMapPipelineSpec.DepthStencilState.DepthTest = true;
        directionalShadowMapPipelineSpec.DepthStencilState.DepthWrite = true;
        directionalShadowMapPipelineSpec.RasterizerState.Cull = CullMode::Front;
        directionalShadowMapPipelineSpec.RasterizerState.DepthBias = -1;
        directionalShadowMapPipelineSpec.RasterizerState.SlopeScaledDepthBias = -1.0f;
        m_DirectionalShadowMapPipeline = Pipeline::Create(directionalShadowMapPipelineSpec);

        FramebufferSpecification pointShadowMapFBSpec{};
        pointShadowMapFBSpec.DebugName = "Point Shadow Map Framebuffer";
        pointShadowMapFBSpec.Attachments = { { TextureFormat::DEPTH_R24G8T, TextureUsage::TextureAttachment, 1u, 1u } };
        pointShadowMapFBSpec.AttachmentsType = TextureType::TextureCube;
        pointShadowMapFBSpec.Width = m_SceneShadowEnvironment.Settings.MapSize;
        pointShadowMapFBSpec.Height = m_SceneShadowEnvironment.Settings.MapSize;
        m_PointShadowMapFramebuffer = Framebuffer::Create(pointShadowMapFBSpec);

        m_SceneShadowEnvironment.PointShadowMaps = AsRef<TextureCube>(m_PointShadowMapFramebuffer->GetDepthAttachment());

        PipelineSpecification pointShadowMapPipelineSpec{};
        pointShadowMapPipelineSpec.DebugName = "Point Shadow Map Pipeline";
        pointShadowMapPipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("OmnidirectionalLightShadowMap");
        pointShadowMapPipelineSpec.TargetFramebuffer = m_PointShadowMapFramebuffer;
        pointShadowMapPipelineSpec.DepthStencilState.CompareOp = CompareOperator::Greater;
        pointShadowMapPipelineSpec.DepthStencilState.DepthTest = true;
        pointShadowMapPipelineSpec.DepthStencilState.DepthWrite = true;
        pointShadowMapPipelineSpec.RasterizerState.Cull = CullMode::Front;
        pointShadowMapPipelineSpec.RasterizerState.DepthBias = -1;
        pointShadowMapPipelineSpec.RasterizerState.SlopeScaledDepthBias = -1.0f;
        m_PointShadowMapPipeline = Pipeline::Create(pointShadowMapPipelineSpec);

        FramebufferSpecification spotShadowMapFBSpec{};
        spotShadowMapFBSpec.DebugName = "Spot Shadow Map Framebuffer";
        spotShadowMapFBSpec.Attachments = { { TextureFormat::DEPTH_R24G8T, TextureUsage::TextureAttachment, 1u, 1u } };
        spotShadowMapFBSpec.Width = m_SceneShadowEnvironment.Settings.MapSize;
        spotShadowMapFBSpec.Height = m_SceneShadowEnvironment.Settings.MapSize;
        m_SpotShadowMapFramebuffer = Framebuffer::Create(spotShadowMapFBSpec);

        m_SceneShadowEnvironment.SpotShadowMaps = AsRef<Texture2D>(m_SpotShadowMapFramebuffer->GetDepthAttachment());

        PipelineSpecification spotShadowMapPipelineSpec{};
        spotShadowMapPipelineSpec.DebugName = "Spot Shadow Map Pipeline";
        spotShadowMapPipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("SpotLightShadowMap");
        spotShadowMapPipelineSpec.TargetFramebuffer = m_SpotShadowMapFramebuffer;
        spotShadowMapPipelineSpec.DepthStencilState.CompareOp = CompareOperator::Greater;
        spotShadowMapPipelineSpec.DepthStencilState.DepthTest = true;
        spotShadowMapPipelineSpec.DepthStencilState.DepthWrite = true;
        spotShadowMapPipelineSpec.RasterizerState.Cull = CullMode::Front;
        spotShadowMapPipelineSpec.RasterizerState.DepthBias = -1;
        spotShadowMapPipelineSpec.RasterizerState.SlopeScaledDepthBias = -1.0f;
        m_SpotShadowMapPipeline = Pipeline::Create(spotShadowMapPipelineSpec);

        BuildIrradianceMap();
        BuildPrefilteredMap();
    }

    void SceneRenderer::PreRender()
    {
        CBLightsCount lightsCount{};
        lightsCount.DirectionalLightsCount = static_cast<uint32_t>(m_Scene->m_LightEnvironment.DirectionalLights.size());
        lightsCount.PointLightsCount = static_cast<uint32_t>(m_Scene->m_LightEnvironment.PointLights.size());
        lightsCount.SpotLightsCount = static_cast<uint32_t>(m_Scene->m_LightEnvironment.SpotLights.size());
        m_CBLightsCount->SetData(Buffer{ &lightsCount, sizeof(CBLightsCount) });
        
        // Binding stuff
        Renderer::SetConstantBuffers(BP_CB_CAMERA, DL_ALL_SHADER_STAGES, { m_CBCamera });
        Renderer::SetConstantBuffers(BP_CB_PBR_SETTINGS, DL_PIXEL_SHADER_BIT, { m_CBPBRSettings });
        Renderer::SetConstantBuffers(BP_CB_SHADOW_MAPPING_DATA, DL_PIXEL_SHADER_BIT, { m_CBShadowMappingData });
        Renderer::SetConstantBuffers(BP_CB_LIGHTS_COUNT, DL_PIXEL_SHADER_BIT, { m_CBLightsCount });

        if (lightsCount.DirectionalLightsCount > 0u)
        {
            BufferViewSpecification bufferViewSpec{};
            bufferViewSpec.FirstElementIndex = 0u;
            bufferViewSpec.ElementCount = lightsCount.DirectionalLightsCount;

            Renderer::SetStructuredBuffers(BP_SB_DIRECTIONAL_LIGHTS, DL_PIXEL_SHADER_BIT, { m_SBDirectionalLights }, { bufferViewSpec });
            Renderer::SetStructuredBuffers(BP_SB_DIRECTIONAL_LIGHTS_POVS, DL_PIXEL_SHADER_BIT, { m_SceneShadowEnvironment.SBDirectionalLightsPOVs }, { bufferViewSpec });
        }

        if (lightsCount.PointLightsCount > 0u)
        {
            BufferViewSpecification bufferViewSpec{};
            bufferViewSpec.FirstElementIndex = 0u;
            bufferViewSpec.ElementCount = lightsCount.PointLightsCount;

            Renderer::SetStructuredBuffers(BP_SB_POINT_LIGHTS, DL_PIXEL_SHADER_BIT, { m_SBPointLights }, { bufferViewSpec });

            bufferViewSpec.ElementCount = lightsCount.PointLightsCount * 6u;
            Renderer::SetStructuredBuffers(BP_SB_POINT_LIGHTS_POVS, DL_PIXEL_SHADER_BIT, { m_SceneShadowEnvironment.SBPointLightsPOVs }, { bufferViewSpec });
        }

        if (lightsCount.SpotLightsCount > 0u)
        {
            BufferViewSpecification bufferViewSpec{};
            bufferViewSpec.FirstElementIndex = 0u;
            bufferViewSpec.ElementCount = lightsCount.SpotLightsCount;

            Renderer::SetStructuredBuffers(BP_SB_SPOT_LIGHTS, DL_PIXEL_SHADER_BIT, { m_SBSpotLights }, { bufferViewSpec });
            Renderer::SetStructuredBuffers(BP_SB_SPOT_LIGHTS_POVS, DL_PIXEL_SHADER_BIT, { m_SceneShadowEnvironment.SBSpotLightsPOVs }, { bufferViewSpec });
        }

        Renderer::SetTextureCubes(BP_TEX_IRRADIANCE_MAP, DL_PIXEL_SHADER_BIT, { m_SceneEnvironment.IrradianceMap }, { TextureViewSpecification{} });
        Renderer::SetTextureCubes(BP_TEX_PREFILTERED_MAP, DL_PIXEL_SHADER_BIT, { m_SceneEnvironment.PrefilteredMap }, { TextureViewSpecification{} });
        Renderer::SetTexture2Ds(BP_TEX_BRDF_LUT, DL_PIXEL_SHADER_BIT, { Renderer::GetBRDFLUT() }, { TextureViewSpecification{} });

        // Setting/updating stuff
        m_Scene->m_MeshRegistry.UpdateInstanceBuffers();
        
        UpdateDirectionalLightsData();
        UpdatePointLightsData();
        UpdateSpotLightsData();

        m_MainFramebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
    }

    void SceneRenderer::ShadowPass()
    {
        constexpr auto submitAllMeshes{ [](const MeshRegistry& meshRegistry, std::string_view shaderName)
            {
                const auto& meshBatch{ meshRegistry.GetMeshBatch(shaderName) };
                for (const auto& [mesh, submeshBatch] : meshBatch.SubmeshBatches)
                {
                    for (uint32_t submeshIndex{ 0u }; submeshIndex < mesh->GetSubmeshes().size(); ++submeshIndex)
                    {
                        for (const auto& [material, instanceBatch] : submeshBatch.MaterialBatches[submeshIndex].InstanceBatches)
                        {
                            const uint32_t instanceCount{ static_cast<uint32_t>(instanceBatch.SubmeshInstances.size()) };
                            Renderer::SubmitStaticMeshInstanced(mesh, submeshIndex, instanceBatch.InstanceBuffer, instanceCount);
                        }
                    }
                }
            }
        };

        const auto& lightsCount{ m_CBLightsCount->GetLocalData().As<CBLightsCount>() };

        // Building directional shadow maps
        Renderer::SetConstantBuffers(BP_CB_NEXT_FREE, DL_VERTEX_SHADER_BIT, { m_SceneShadowEnvironment.CBDirectionalLightData });
        for (uint32_t i{ 0u }; i < lightsCount->DirectionalLightsCount; ++i)
        {
            const auto& directionalLightData{ m_SceneShadowEnvironment.DirectionalLightsData[i] };

            CBDirectionalLightShadowMapData directionalLightShadowData{};
            directionalLightShadowData.LightDirection = m_Scene->m_LightEnvironment.DirectionalLights[i].Direction;
            directionalLightShadowData.WorldTexelSize = directionalLightData.WorldTexelSize;
            m_SceneShadowEnvironment.CBDirectionalLightData->SetData(Buffer{ &directionalLightShadowData, sizeof(CBDirectionalLightShadowMapData) });

            UpdateCBCamera(directionalLightData.POV);

            TextureViewSpecification depthAttachmentViewSpec{};
            depthAttachmentViewSpec.Format = TextureFormat::DEPTH24STENCIL8;
            depthAttachmentViewSpec.Subresource.BaseMip = 0u;
            depthAttachmentViewSpec.Subresource.MipsCount = 1u;
            depthAttachmentViewSpec.Subresource.BaseLayer = i;
            depthAttachmentViewSpec.Subresource.LayersCount = 1u;
            m_DirectionalShadowMapFramebuffer->SetDepthAttachmentViewSpecification(depthAttachmentViewSpec);

            Renderer::SetPipeline(m_DirectionalShadowMapPipeline, true);
            submitAllMeshes(m_Scene->m_MeshRegistry, "PBR_Static");
        }

        // Building point shadow maps
        Renderer::SetConstantBuffers(BP_CB_NEXT_FREE, DL_VERTEX_SHADER_BIT | DL_GEOMETRY_SHADER_BIT, { m_SceneShadowEnvironment.CBPointLightData });
        for (uint32_t i{ 0u }; i < lightsCount->PointLightsCount; ++i)
        {
            const auto& pointLightData{ m_SceneShadowEnvironment.PointLightsData[i] };

            CBOmnidirectionalLightShadowData pointLightShadowData{};
            for (uint32_t face{ 0u }; face < 6u; ++face)
            {
                const auto& facePOV{ pointLightData.POVs[face] };
                pointLightShadowData.LightViewProjections[face] = facePOV.GetViewMatrix() * facePOV.GetProjectionMatrix();
            }
            pointLightShadowData.LightPosition = pointLightData.POVs[0].GetPosition();
            pointLightShadowData.WorldTexelSize = pointLightData.WorldTexelSize;
            m_SceneShadowEnvironment.CBPointLightData->SetData(Buffer{ &pointLightShadowData, sizeof(CBOmnidirectionalLightShadowData) });

            TextureViewSpecification depthAttachmentViewSpec{};
            depthAttachmentViewSpec.Format = TextureFormat::DEPTH24STENCIL8;
            depthAttachmentViewSpec.Subresource.BaseMip = 0u;
            depthAttachmentViewSpec.Subresource.MipsCount = 1u;
            depthAttachmentViewSpec.Subresource.BaseLayer = i;
            depthAttachmentViewSpec.Subresource.LayersCount = 1u;
            m_PointShadowMapFramebuffer->SetDepthAttachmentViewSpecification(depthAttachmentViewSpec);

            Renderer::SetPipeline(m_PointShadowMapPipeline, true);
            submitAllMeshes(m_Scene->m_MeshRegistry, "PBR_Static");
        }

        // Building spot shadow maps
        Renderer::SetConstantBuffers(BP_CB_NEXT_FREE, DL_VERTEX_SHADER_BIT, { m_SceneShadowEnvironment.CBSpotLightData });
        for (uint32_t i{ 0u }; i < lightsCount->SpotLightsCount; ++i)
        {
            const auto& spotLightData{ m_SceneShadowEnvironment.SpotLightsData[i] };

            CBSpotLightShadowMapData spotLightShadowData{};
            spotLightShadowData.LightPosition = spotLightData.POV.GetPosition();
            spotLightShadowData.WorldTexelSize = spotLightData.WorldTexelSize;
            m_SceneShadowEnvironment.CBSpotLightData->SetData(Buffer{ &spotLightShadowData, sizeof(CBSpotLightShadowMapData) });

            UpdateCBCamera(spotLightData.POV);

            TextureViewSpecification depthAttachmentViewSpec{};
            depthAttachmentViewSpec.Format = TextureFormat::DEPTH24STENCIL8;
            depthAttachmentViewSpec.Subresource.BaseMip = 0u;
            depthAttachmentViewSpec.Subresource.MipsCount = 1u;
            depthAttachmentViewSpec.Subresource.BaseLayer = i;
            depthAttachmentViewSpec.Subresource.LayersCount = 1u;
            m_SpotShadowMapFramebuffer->SetDepthAttachmentViewSpecification(depthAttachmentViewSpec);

            Renderer::SetPipeline(m_SpotShadowMapPipeline, true);
            submitAllMeshes(m_Scene->m_MeshRegistry, "PBR_Static");
        }
    }

    void SceneRenderer::GeometryPass()
    {
        constexpr auto submitAllMeshesWithMaterial{ [](const MeshRegistry& meshRegistry, std::string_view shaderName)
            {
                const auto& meshBatch{ meshRegistry.GetMeshBatch(shaderName) };
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
        };

        const auto& sceneCamera{ m_Scene->m_SceneCameraController.GetCamera() };
        UpdateCBCamera(sceneCamera);

        const auto& lightsCount{ m_CBLightsCount->GetLocalData().As<CBLightsCount>() };

        Renderer::SetPipeline(m_PBRStaticPipeline, true);
        
        TextureViewSpecification directionalShadowMaps{};
        directionalShadowMaps.Format = TextureFormat::R24X8;
        directionalShadowMaps.Subresource.BaseMip = 0u;
        directionalShadowMaps.Subresource.MipsCount = 1u;
        directionalShadowMaps.Subresource.BaseLayer = 0u;
        directionalShadowMaps.Subresource.LayersCount = lightsCount->DirectionalLightsCount;
        Renderer::SetTexture2Ds(BP_TEX_DIRECTIONAL_SHADOW_MAPS, DL_PIXEL_SHADER_BIT, { m_SceneShadowEnvironment.DirectionalShadowMaps }, { directionalShadowMaps });
        
        TextureViewSpecification pointShadowMaps{};
        pointShadowMaps.Format = TextureFormat::R24X8;
        pointShadowMaps.Subresource.BaseMip = 0u;
        pointShadowMaps.Subresource.MipsCount = 1u;
        pointShadowMaps.Subresource.BaseLayer = 0u;
        pointShadowMaps.Subresource.LayersCount = lightsCount->PointLightsCount;
        Renderer::SetTextureCubes(BP_TEX_POINT_SHADOW_MAPS, DL_PIXEL_SHADER_BIT, { m_SceneShadowEnvironment.PointShadowMaps }, { pointShadowMaps });

        TextureViewSpecification spotShadowMaps{};
        spotShadowMaps.Format = TextureFormat::R24X8;
        spotShadowMaps.Subresource.BaseMip = 0u;
        spotShadowMaps.Subresource.MipsCount = 1u;
        spotShadowMaps.Subresource.BaseLayer = 0u;
        spotShadowMaps.Subresource.LayersCount = lightsCount->SpotLightsCount;
        Renderer::SetTexture2Ds(BP_TEX_SPOT_SHADOW_MAPS, DL_PIXEL_SHADER_BIT, { m_SceneShadowEnvironment.SpotShadowMaps }, { spotShadowMaps });

        submitAllMeshesWithMaterial(m_Scene->m_MeshRegistry, "PBR_Static");

        Renderer::SetPipeline(m_EmissionPipeline, false);
        submitAllMeshesWithMaterial(m_Scene->m_MeshRegistry, "Emission");
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
        cameraData.Projection        = camera.GetProjectionMatrix();
        cameraData.InvProjection     = Math::Mat4x4::Inverse(cameraData.Projection);
        cameraData.View              = camera.GetViewMatrix();
        cameraData.InvView           = Math::Mat4x4::Inverse(cameraData.View);
        cameraData.ViewProjection    = cameraData.View * cameraData.Projection;
        cameraData.InvViewProjection = Math::Mat4x4::Inverse(cameraData.ViewProjection);
        
        cameraData.CameraPosition    = camera.GetPosition();
        
        cameraData.BL    = camera.ConstructFrustumPosNoTranslation(Math::Vec3{ -1.0f, -1.0f, 1.0f });
        cameraData.BL2BR = camera.ConstructFrustumPosNoTranslation(Math::Vec3{  1.0f, -1.0f, 1.0f }) - cameraData.BL;
        cameraData.BL2TL = camera.ConstructFrustumPosNoTranslation(Math::Vec3{ -1.0f,  1.0f, 1.0f }) - cameraData.BL;

        m_CBCamera->SetData(Buffer{ &cameraData, sizeof(CBCamera) });
    }

    void SceneRenderer::UpdateDirectionalLightsData()
    {
        const auto& directionalLightsCount{ m_CBLightsCount->GetLocalData().As<CBLightsCount>()->DirectionalLightsCount };

        // Recreate directional lights structured buffer if needed
        if (m_SBDirectionalLights->GetElementsCount() < directionalLightsCount)
            m_SBDirectionalLights = StructuredBuffer::Create(sizeof(DirectionalLight), directionalLightsCount);

        if (m_SceneShadowEnvironment.SBDirectionalLightsPOVs->GetElementsCount() < directionalLightsCount)
            m_SceneShadowEnvironment.SBDirectionalLightsPOVs = StructuredBuffer::Create(sizeof(Math::Mat4x4), directionalLightsCount);

        // Recreate directional shadow map framebuffer if needed
        if (m_DirectionalShadowMapFramebuffer->GetDepthAttachment()->GetLayersCount() < directionalLightsCount ||
            m_DirectionalShadowMapFramebuffer->GetSpecification().Width != m_SceneShadowEnvironment.Settings.MapSize)
        {
            FramebufferSpecification directionalShadowMapFBSpec{};
            directionalShadowMapFBSpec.DebugName = "Directional Shadow Map Framebuffer";
            directionalShadowMapFBSpec.Attachments = { { TextureFormat::DEPTH_R24G8T, TextureUsage::TextureAttachment, 1u, directionalLightsCount } };
            directionalShadowMapFBSpec.Width = m_SceneShadowEnvironment.Settings.MapSize;
            directionalShadowMapFBSpec.Height = m_SceneShadowEnvironment.Settings.MapSize;
            m_DirectionalShadowMapFramebuffer = Framebuffer::Create(directionalShadowMapFBSpec);

            m_SceneShadowEnvironment.DirectionalShadowMaps = AsRef<Texture2D>(m_DirectionalShadowMapFramebuffer->GetDepthAttachment());

            m_DirectionalShadowMapPipeline->SetFramebuffer(m_DirectionalShadowMapFramebuffer);
        }

        const bool lightEnvironmentHasChanged{ static_cast<uint32_t>(m_SceneShadowEnvironment.DirectionalLightsData.size()) > directionalLightsCount };

        m_SceneShadowEnvironment.DirectionalLightsData.resize(directionalLightsCount);

        const auto& sceneCamera{ m_Scene->m_SceneCameraController.GetCamera() };
        auto sceneCameraFrustum{ sceneCamera.ConstructFrustum() };

        // Account for shadow distance
        const auto& sceneCameraForward{ sceneCamera.GetForward() };
        const float shadowDistance{ m_SceneShadowEnvironment.Settings.DirectionalLightShadowDistance };
        Math::Plane newFarPlane{ sceneCamera.GetPosition() + shadowDistance * sceneCameraForward, sceneCameraForward };
        for (uint32_t i{ 0u }; i < 4u; ++i) // Traverse plane corners
        {
            const auto& nearPlaneFrustumPos{ sceneCameraFrustum.Corners[i] };
            auto& farPlaneFrustumPos{ sceneCameraFrustum.Corners[i + 4u] };

            Math::Ray ray{ nearPlaneFrustumPos, Math::Normalize(farPlaneFrustumPos - nearPlaneFrustumPos) };
            Math::IntersectInfo intersectInfo{};
            Math::Intersects(ray, newFarPlane, intersectInfo);

            farPlaneFrustumPos = intersectInfo.IntersectionPoint;
        }

        // Calculate scene camera frustum center
        Math::Vec3 sceneCameraFrustumCenter{ 0.0f };
        for (const auto& frustumPos : sceneCameraFrustum.Corners)
            sceneCameraFrustumCenter += frustumPos;
        sceneCameraFrustumCenter /= 8.0f;
        
        const auto& sceneCameraPos{ sceneCamera.GetPosition() };

        auto directionalLightsSB{ m_SBDirectionalLights->Map().As<DirectionalLight>() };
        auto directionalLightsPOVsSB{ m_SceneShadowEnvironment.SBDirectionalLightsPOVs->Map().As<Math::Mat4x4>() };
        for (uint32_t i{ 0u }; i < directionalLightsCount; ++i)
        {
            const auto& directionalLight{ m_Scene->m_LightEnvironment.DirectionalLights[i] };

            // Upload directional light data to the structured buffer
            directionalLightsSB[i] = directionalLight;

            // Update directional light view and projection matrices
            {
                auto& directionalLightData{ m_SceneShadowEnvironment.DirectionalLightsData[i] };

                const auto& prevLightView{ directionalLightData.POV.GetViewMatrix() };

                const auto& sceneCameraPosLightPOV{ Math::PointToSpace(sceneCameraPos, prevLightView) };
                const auto& prevSceneCameraPosLightPOV{ Math::PointToSpace(directionalLightData.PrevSceneCameraPosition, prevLightView) };
                
                const auto& cameraPosDeltaSizeX{ sceneCameraPosLightPOV.y - prevSceneCameraPosLightPOV.x };
                const auto& cameraPosDeltaSizeY{ sceneCameraPosLightPOV.y - prevSceneCameraPosLightPOV.y };
                const auto& cameraPosDeltaSizeZ{ sceneCameraPosLightPOV.z - prevSceneCameraPosLightPOV.z };
                const bool cameraDeltaPosGreaterThanTexelWorldSize{ cameraPosDeltaSizeX > directionalLightData.WorldTexelSize &&
                    cameraPosDeltaSizeY > directionalLightData.WorldTexelSize &&
                    cameraPosDeltaSizeZ > directionalLightData.WorldTexelSize
                };

                const bool lightDataNotInitialized{ directionalLightData.WorldTexelSize == Math::Numeric::Inf };

                const bool shouldUpdateShadowData{
                    cameraDeltaPosGreaterThanTexelWorldSize ||
                    lightDataNotInitialized ||
                    lightEnvironmentHasChanged ||
                    m_SceneShadowEnvironment.ForceRecreateDirectionalLightShadowMaps
                };

                if (!shouldUpdateShadowData)
                {
                    // Need to transpose as matrices are expected as column-major in HLSL
                    directionalLightsPOVsSB[i] = Math::Mat4x4::Transpose(prevLightView * directionalLightData.POV.GetProjectionMatrix());
                    continue;
                }

                m_SceneShadowEnvironment.ForceRecreateDirectionalLightShadowMaps = false;
                
                directionalLightData.PrevSceneCameraPosition = sceneCameraPos;

                // Calculate view matrix
                const auto& lightViewForward{ directionalLight.Direction };
                Math::Vec3 lightViewRight, lightViewUp;
                Math::BranchlessONB(lightViewForward, lightViewRight, lightViewUp);

                directionalLightData.POV.SetView(sceneCameraFrustumCenter, lightViewForward, lightViewRight, lightViewUp);

                // Calculate projection matrix
                const auto& lightPOV{ directionalLightData.POV.GetViewMatrix() };
                std::array<Math::Vec3, 8> lightPOVFrustumCorners{};

                Math::Vec3 lightPOVFrustumNearZ{ Math::Numeric::Max }, lightPOVFrustumFarZ{ -Math::Numeric::Max };
                for (uint32_t cornerIndex{ 0u }; cornerIndex < sceneCameraFrustum.Corners.size(); ++cornerIndex)
                {
                    lightPOVFrustumCorners[cornerIndex] = Math::PointToSpace(sceneCameraFrustum.Corners[cornerIndex], lightPOV);
                    lightPOVFrustumNearZ = Math::Min(lightPOVFrustumNearZ, lightPOVFrustumCorners[cornerIndex]);
                    lightPOVFrustumFarZ = Math::Max(lightPOVFrustumFarZ, lightPOVFrustumCorners[cornerIndex]);
                }

                Math::Vec3 lightPOVFrustumNearMinXY{ Math::Numeric::Max }, lightPOVFrustumNearMaxXY{ -Math::Numeric::Max };
                for (const auto& lightPOVFrustumCorner : lightPOVFrustumCorners)
                {
                    const Math::Vec3 planOriginToCorner{ lightPOVFrustumCorner - lightPOVFrustumNearZ };
                    const float distanceToLightPOVNearPlane{ Math::Dot(lightViewForward, planOriginToCorner) };
                    const Math::Vec3 projectedFrustumCorner{ lightPOVFrustumCorner - distanceToLightPOVNearPlane * lightViewForward };

                    lightPOVFrustumNearMinXY = Math::Min(lightPOVFrustumNearMinXY, projectedFrustumCorner);
                    lightPOVFrustumNearMaxXY = Math::Max(lightPOVFrustumNearMaxXY, projectedFrustumCorner);
                }

                const auto& lightPOVFrustumSize{ Math::Length(lightPOVFrustumNearMaxXY - lightPOVFrustumNearMinXY) };

                const auto& distanceToLightPOVNearPlane{ Math::Dot(Math::Vec3{ 0.0f, 0.0f, 1.0f }, lightPOVFrustumNearZ) };
                const auto& distanceToLightPOVFarPlane{ Math::Dot(Math::Vec3{ 0.0f, 0.0f, 1.0f }, lightPOVFrustumFarZ) };

                directionalLightData.POV.SetOrthographicProjection(
                    lightPOVFrustumSize,
                    lightPOVFrustumSize,
                    distanceToLightPOVFarPlane,
                    distanceToLightPOVNearPlane - m_SceneShadowEnvironment.Settings.DirectionalLightShadowMargin
                );

                // Calculate texel size
                directionalLightData.WorldTexelSize = lightPOVFrustumSize / m_SceneShadowEnvironment.Settings.MapSize;

                // Need to transpose as matrices are expected as column-major in HLSL
                directionalLightsPOVsSB[i] = Math::Mat4x4::Transpose(lightPOV * directionalLightData.POV.GetProjectionMatrix());
            }
        }
        m_SceneShadowEnvironment.SBDirectionalLightsPOVs->Unmap();
        m_SBDirectionalLights->Unmap();
    }

    void SceneRenderer::UpdatePointLightsData()
    {
        const auto& pointLightsCount{ m_CBLightsCount->GetLocalData().As<CBLightsCount>()->PointLightsCount };
     
        // Recreate point lights structured buffer if needed
        if (m_SBPointLights->GetElementsCount() < pointLightsCount)
            m_SBPointLights = StructuredBuffer::Create(sizeof(PointLight), pointLightsCount);

        if (m_SceneShadowEnvironment.SBPointLightsPOVs->GetElementsCount() < pointLightsCount * 6u)
            m_SceneShadowEnvironment.SBPointLightsPOVs = StructuredBuffer::Create(sizeof(Math::Mat4x4), pointLightsCount * 6u);

        // Recreate point shadow map framebuffer if needed
        if (m_PointShadowMapFramebuffer->GetDepthAttachment()->GetLayersCount() < pointLightsCount ||
            m_PointShadowMapFramebuffer->GetSpecification().Width != m_SceneShadowEnvironment.Settings.MapSize)
        {
            FramebufferSpecification pointShadowMapFBSpec{};
            pointShadowMapFBSpec.DebugName = "Point Shadow Map Framebuffer";
            pointShadowMapFBSpec.Attachments = { { TextureFormat::DEPTH_R24G8T, TextureUsage::TextureAttachment, 1u, pointLightsCount } };
            pointShadowMapFBSpec.AttachmentsType = TextureType::TextureCube;
            pointShadowMapFBSpec.Width = m_SceneShadowEnvironment.Settings.MapSize;
            pointShadowMapFBSpec.Height = m_SceneShadowEnvironment.Settings.MapSize;
            m_PointShadowMapFramebuffer = Framebuffer::Create(pointShadowMapFBSpec);

            m_SceneShadowEnvironment.PointShadowMaps = AsRef<TextureCube>(m_PointShadowMapFramebuffer->GetDepthAttachment());

            m_PointShadowMapPipeline->SetFramebuffer(m_PointShadowMapFramebuffer);
        }

        m_SceneShadowEnvironment.PointLightsData.resize(pointLightsCount);

        auto pointLightsSB{ m_SBPointLights->Map().As<PointLight>() };
        auto pointLightsPOVsSB{ m_SceneShadowEnvironment.SBPointLightsPOVs->Map().As<Math::Mat4x4>() };
        for (uint32_t i{ 0u }; i < pointLightsCount; ++i)
        {
            auto& [light, instance] { m_Scene->m_LightEnvironment.PointLights[i] };

            const auto& transform{ instance->Get<Math::Mat4x4>("TRANSFORM") };

            PointLight transformedLight{ light };
            transformedLight.Position = Math::PointToSpace(light.Position, transform);

            // Upload point light data to the structured buffer
            pointLightsSB[i] = transformedLight;

            // Update point light view and projection matrices
            {
                auto& pointLightData{ m_SceneShadowEnvironment.PointLightsData[i] };

                // Calculate view matrices and projection matrix
                static constexpr std::array<Math::Vec3, 6u> cubeDirections{
                    Math::Vec3{  1.0f,  0.0f,  0.0f }, // +X
                    Math::Vec3{ -1.0f,  0.0f,  0.0f }, // -X
                    Math::Vec3{  0.0f,  1.0f,  0.0f }, // +Y
                    Math::Vec3{  0.0f, -1.0f,  0.0f }, // -Y
                    Math::Vec3{  0.0f,  0.0f,  1.0f }, // +Z
                    Math::Vec3{  0.0f,  0.0f, -1.0f }  // -Z
                };

                const float nearZ{ Utils::SphereLightContributionDistance(1e-5f, transformedLight.Radius) };
                const float farZ{ transformedLight.Radius };

                for (uint32_t face{ 0u }; face < 5u; ++face)
                {
                    const auto& lightViewForward{ cubeDirections[face] };
                    Math::Vec3 lightViewRight, lightViewUp;
                    Math::BranchlessONB(lightViewForward, lightViewRight, lightViewUp);

                    pointLightData.POVs[face].SetView(transformedLight.Position, lightViewForward, lightViewUp, lightViewRight);
                    pointLightData.POVs[face].SetPerspectiveProjectionFov(Math::Numeric::Pi / 2.0f, 1.0f, nearZ,farZ);

                    const auto& povView{ pointLightData.POVs[face].GetViewMatrix() };
                    const auto& povProjection{ pointLightData.POVs[face].GetProjectionMatrix() };

                    // Need to transpose as matrices are expected as column-major in HLSL
                    pointLightsPOVsSB[i * 6u + face] = Math::Mat4x4::Transpose(povView * povProjection);
                }

                // When rotating for -Z face, BranchlessONB gives the basis that has mirrored in the X ans Y axes
                {
                    const auto& lightViewForward{ cubeDirections[5u] };
                    Math::Vec3 lightViewRight, lightViewUp;
                    Math::BranchlessONB(lightViewForward, lightViewRight, lightViewUp);

                    pointLightData.POVs[5].SetView(transformedLight.Position, lightViewForward, -lightViewUp, -lightViewRight);
                    pointLightData.POVs[5].SetPerspectiveProjectionFov(Math::Numeric::Pi / 2.0f, 1.0f, nearZ, farZ);

                    const auto& povView{ pointLightData.POVs[5u].GetViewMatrix() };
                    const auto& povProjection{ pointLightData.POVs[5u].GetProjectionMatrix() };

                    // Need to transpose as matrices are expected as column-major in HLSL
                    pointLightsPOVsSB[i * 6u + 5u] = Math::Mat4x4::Transpose(povView * povProjection);
                }

                // Calculate texel size
                const auto& lightPOV0{ pointLightData.POVs[0] };
                const auto& lightPOV0TopLeft{ lightPOV0.ConstructFrustumPos(Math::Vec3{ -1.0f, 1.0f, 1.0f }) };
                const auto& lightPOV0TopRight{ lightPOV0.ConstructFrustumPos(Math::Vec3{ 1.0f, 1.0f, 1.0f }) };

                const auto& lightPOV0Size{ Math::Length(lightPOV0TopRight - lightPOV0TopLeft) };

                pointLightData.WorldTexelSize = lightPOV0Size / m_SceneShadowEnvironment.Settings.MapSize;
            }
        }
        m_SceneShadowEnvironment.SBPointLightsPOVs->Unmap();
        m_SBPointLights->Unmap();
    }

    void SceneRenderer::UpdateSpotLightsData()
    {
        const auto& spotLightsCount{ m_CBLightsCount->GetLocalData().As<CBLightsCount>()->SpotLightsCount };
        
        // Recreate spot lights structured buffer if needed
        if (m_SBSpotLights->GetElementsCount() < spotLightsCount)
            m_SBSpotLights = StructuredBuffer::Create(sizeof(SpotLight), spotLightsCount);

        if (m_SceneShadowEnvironment.SBSpotLightsPOVs->GetElementsCount() < spotLightsCount)
            m_SceneShadowEnvironment.SBSpotLightsPOVs = StructuredBuffer::Create(sizeof(Math::Mat4x4), spotLightsCount);

        // Recreate spot shadow map framebuffer if needed
        if (m_SpotShadowMapFramebuffer->GetDepthAttachment()->GetLayersCount() < spotLightsCount ||
            m_SpotShadowMapFramebuffer->GetSpecification().Width != m_SceneShadowEnvironment.Settings.MapSize)
        {
            FramebufferSpecification spotShadowMapFBSpec{};
            spotShadowMapFBSpec.DebugName = "Spot Shadow Map Framebuffer";
            spotShadowMapFBSpec.Attachments = { { TextureFormat::DEPTH_R24G8T, TextureUsage::TextureAttachment, 1u, spotLightsCount } };
            spotShadowMapFBSpec.Width = m_SceneShadowEnvironment.Settings.MapSize;
            spotShadowMapFBSpec.Height = m_SceneShadowEnvironment.Settings.MapSize;
            m_SpotShadowMapFramebuffer = Framebuffer::Create(spotShadowMapFBSpec);

            m_SceneShadowEnvironment.SpotShadowMaps = AsRef<Texture2D>(m_SpotShadowMapFramebuffer->GetDepthAttachment());

            m_SpotShadowMapPipeline->SetFramebuffer(m_SpotShadowMapFramebuffer);
        }

        m_SceneShadowEnvironment.SpotLightsData.resize(spotLightsCount);

        auto spotLightsSB{ m_SBSpotLights->Map().As<SpotLight>() };
        auto spotLightsPOVsSB{ m_SceneShadowEnvironment.SBSpotLightsPOVs->Map().As<Math::Mat4x4>() };
        for (uint32_t i{ 0u }; i < spotLightsCount; ++i)
        {
            auto& [light, instance] { m_Scene->m_LightEnvironment.SpotLights[i] };

            const auto& transform{ instance->Get<Math::Mat4x4>("TRANSFORM") };

            SpotLight transformedLight{ light };
            transformedLight.Position = Math::PointToSpace(light.Position, transform);
            transformedLight.Direction = Math::Normalize(Math::DirectionToSpace(light.Direction, transform));

            spotLightsSB[i] = transformedLight;
            
            // Update spot light view and projection matrices
            {
                auto& spotLightData{ m_SceneShadowEnvironment.SpotLightsData[i] };

                const auto& lightViewForward{ transformedLight.Direction };
                Math::Vec3 lightViewRight, lightViewUp;
                Math::BranchlessONB(lightViewForward, lightViewRight, lightViewUp);

                spotLightData.POV.SetView(transformedLight.Position, lightViewForward, lightViewUp, lightViewRight);
                
                const float nearZ{ Utils::SphereLightContributionDistance(1e-5f, transformedLight.Radius) };
                const float farZ{ transformedLight.Radius };

                spotLightData.POV.SetPerspectiveProjectionFov(Math::Acos(transformedLight.OuterCutoffCos) * 2.0f, 1.0f, nearZ, farZ);

                // Need to transpose as matrices are expected as column-major in HLSL
                spotLightsPOVsSB[i] = Math::Mat4x4::Transpose(spotLightData.POV.GetViewMatrix() * spotLightData.POV.GetProjectionMatrix());

                // Calculate texel size
                const auto& lightPOV{ spotLightData.POV };
                const auto& lightPOVTopLeft{ lightPOV.ConstructFrustumPos(Math::Vec3{ -1.0f, 1.0f, 1.0f }) };
                const auto& lightPOVTopRight{ lightPOV.ConstructFrustumPos(Math::Vec3{ 1.0f, 1.0f, 1.0f }) };

                const auto& lightPOVSize{ Math::Length(lightPOVTopRight - lightPOVTopLeft) };

                spotLightData.WorldTexelSize = lightPOVSize / m_SceneShadowEnvironment.Settings.MapSize;
            }
        }
        m_SceneShadowEnvironment.SBSpotLightsPOVs->Unmap();
        m_SBSpotLights->Unmap();
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
        irradianceMapSpec.Layers = 1u;
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
        prefilteredMapSpec.Layers = 1u;
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
            framebufferViewSpec.Subresource.LayersCount = 1u;
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