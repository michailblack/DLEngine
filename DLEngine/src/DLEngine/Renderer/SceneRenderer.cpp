#include "dlpch.h"
#include "SceneRenderer.h"

#include "DLEngine/Math/Intersections.h"

#include "DLEngine/Renderer/Renderer.h"

namespace DLEngine
{
    namespace Utils
    {
        namespace
        {
            void SubmitMeshBatch(const MeshRegistry& meshRegistry, std::string_view shaderName, bool setMaterial)
            {
                const auto& meshBatch{ meshRegistry.GetMeshBatch(shaderName) };
                for (const auto& [mesh, submeshBatch] : meshBatch.SubmeshBatches)
                {
                    for (uint32_t submeshIndex{ 0u }; submeshIndex < mesh->GetSubmeshes().size(); ++submeshIndex)
                    {
                        for (const auto& [material, instanceBatch] : submeshBatch.MaterialBatches[submeshIndex].InstanceBatches)
                        {
                            if (setMaterial)
                                Renderer::SetMaterial(material);
                            
                            const uint32_t instanceCount{ static_cast<uint32_t>(instanceBatch.SubmeshInstances.size()) };
                            Renderer::SubmitStaticMeshInstanced(mesh, submeshIndex, instanceBatch.InstanceBuffers, instanceCount);
                        }
                    }
                }
            }
        }
    }

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

            // G-Buffer textures
            BP_TEX_GBUFFER_ALBEDO                   = 16u,
            BP_TEX_GBUFFER_METALNESS_ROUGHNESS      = 17u,
            BP_TEX_GBUFFER_GEOMETRY_SURFACE_NORMALS = 18u,
            BP_TEX_GBUFFER_EMISSION                 = 19u,

            BP_TEX_NEXT_FREE,
        };

        struct CBCamera
        {
            Math::Mat4x4 Projection;
            Math::Mat4x4 InvProjection;
            Math::Mat4x4 View;
            Math::Mat4x4 InvView;
            Math::Mat4x4 ViewProjection;
            Math::Mat4x4 InvViewProjection;
            Math::Vec3 CameraPosition;
            float zNear;
            Math::Vec3 BL;
            float zFar;
            Math::Vec3 BL2TL;
            float _padding1{ 0.0f };
            Math::Vec3 BL2BR;
            float _padding2{ 0.0f };
        };

        struct CBLightsCount
        {
            uint32_t DirectionalLightsCount{ 0u };
            uint32_t PointLightsCount{ 0u };
            uint32_t SpotLightsCount{ 0u };
            float _padding{ 0.0f };
        };

        struct CBPBRSettings
        {
            Math::Vec3 IndirectLightRadiance{ 0.1f };
            float OverwrittenRoughness{ 0.0f };
            uint32_t OverwriteRoughness{ static_cast<uint32_t>(false) };
            uint32_t UseIBL{ static_cast<uint32_t>(true) };
            uint32_t UseDiffuseReflections{ static_cast<uint32_t>(true) };
            uint32_t UseSpecularReflections{ static_cast<uint32_t>(true) };
        };

        struct CBPostProcessingSettings
        {
            float EV100{ 0.0f };
            float Gamma{ 2.2f };
            float _padding[2u]{};
        };

        struct CBOmnidirectionalLightShadowData
        {
            std::array<Math::Mat4x4, 6u> LightViewProjections;
        };

        struct CBShadowMappingData
        {
            uint32_t ShadowMapSize;
            float ShadowBias;
            uint32_t UseDirectionalShadows{ static_cast<uint32_t>(true) };
            uint32_t UseOmnidirectionalShadows{ static_cast<uint32_t>(true) };
            uint32_t UseSpotShadows{ static_cast<uint32_t>(true) };
            uint32_t UsePCF{ static_cast<uint32_t>(true) };
            float _padding[2u];
        };

        struct VBSmokeParticle
        {
            Math::Vec3 WorldPosition;
            Math::Vec3 TintColor;
            Math::Vec2 InitialSize;
            Math::Vec2 EndSize;
            float EmissionIntensity;
            float LifetimeMS;
            float LifetimePassedMS;
            float Rotation;
        };

        struct CBTextureAtlasData
        {
            Math::Vec2 Size;
            Math::Vec2 TileSize;
            uint32_t TilesCountX;
            uint32_t TilesCountY;
            float _padding[2u];
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
        m_ViewportWidth = m_Scene->m_ViewportWidth;
        m_ViewportHeight = m_Scene->m_ViewportHeight;

        PreRender();

        ShadowPass();
        GBufferPass();
        FullscreenPass();
        SkyboxPass();
        SmokeParticlesPass();
        PostProcessPass();
    }

    void SceneRenderer::SetPBRSettings(const PBRSettings& pbrSettings)
    {
        CBPBRSettings pbrSettingsData{};
        pbrSettingsData.IndirectLightRadiance = pbrSettings.IndirectLightRadiance;
        pbrSettingsData.OverwrittenRoughness = pbrSettings.OverwrittenRoughness;
        pbrSettingsData.OverwriteRoughness = static_cast<uint32_t>(pbrSettings.OverwriteRoughness);
        pbrSettingsData.UseIBL = static_cast<uint32_t>(pbrSettings.UseIBL);
        pbrSettingsData.UseDiffuseReflections = static_cast<uint32_t>(pbrSettings.UseDiffuseReflections);
        pbrSettingsData.UseSpecularReflections = static_cast<uint32_t>(pbrSettings.UseSpecularReflections);

        m_CBPBRSettings->SetData(Buffer{ &pbrSettingsData, sizeof(CBPBRSettings) });
    }

    void SceneRenderer::SetPostProcessingSettings(const PostProcessingSettings& postProcessingSettings)
    {
        CBPostProcessingSettings postProcessingSettingsData{};
        postProcessingSettingsData.EV100 = postProcessingSettings.EV100;
        postProcessingSettingsData.Gamma = postProcessingSettings.Gamma;

        m_CBPostProcessSettings->SetData(Buffer{ &postProcessingSettingsData, sizeof(CBPostProcessingSettings) });
    }

    void SceneRenderer::SetShadowMappingSettings(const ShadowMappingSettings& shadowMapSettings)
    {
        m_SceneShadowEnvironment.ForceRecreateDirectionalLightShadowMaps = m_SceneShadowEnvironment.Settings.MapSize != shadowMapSettings.MapSize ||
            m_SceneShadowEnvironment.Settings.DirectionalLightShadowMargin != shadowMapSettings.DirectionalLightShadowMargin ||
            m_SceneShadowEnvironment.Settings.DirectionalLightShadowDistance != shadowMapSettings.DirectionalLightShadowDistance;

        m_SceneShadowEnvironment.Settings = shadowMapSettings;

        CBShadowMappingData shadowMappingData{};
        shadowMappingData.ShadowMapSize = shadowMapSettings.MapSize;
        shadowMappingData.ShadowBias = shadowMapSettings.ShadowBias;
        shadowMappingData.UseDirectionalShadows = static_cast<uint32_t>(shadowMapSettings.UseDirectionalShadows);
        shadowMappingData.UseOmnidirectionalShadows = static_cast<uint32_t>(shadowMapSettings.UseOmnidirectionalShadows);
        shadowMappingData.UseSpotShadows = static_cast<uint32_t>(shadowMapSettings.UseSpotShadows);
        shadowMappingData.UsePCF = static_cast<uint32_t>(shadowMapSettings.UsePCF);
        m_CBShadowMappingData->SetData(Buffer{ &shadowMappingData, sizeof(CBShadowMappingData) });
    }

    void SceneRenderer::Init()
    {
        InitBuffers();
        InitFramebuffers();
        InitPipelines();

#if 0
        PipelineSpecification smokeParticlePipelineSpec{};
        const auto& smokeParticleShader{ Renderer::GetShaderLibrary()->Get("SmokeParticle") };

        smokeParticlePipelineSpec.DebugName = "Smoke Particle Pipeline";
        smokeParticlePipelineSpec.Shader = smokeParticleShader;
        smokeParticlePipelineSpec.TargetFramebuffer = m_HDRFramebuffer;
        smokeParticlePipelineSpec.DepthStencilState.DepthCompareOp = CompareOperator::Greater;
        smokeParticlePipelineSpec.DepthStencilState.DepthTest = true;
        smokeParticlePipelineSpec.DepthStencilState.DepthWrite = false;
        smokeParticlePipelineSpec.RasterizerState.Cull = CullMode::Back;
        smokeParticlePipelineSpec.BlendState = BlendState::General;
        m_SmokeParticlePipeline = Pipeline::Create(smokeParticlePipelineSpec);

        m_SmokeParticlesInstanceBuffer = VertexBuffer::Create(smokeParticleShader->GetInstanceLayout(), sizeof(VBSmokeParticle));

        // Load smoke particle 6-way light maps
        auto textureLibrary{ DLEngine::Renderer::GetTextureLibrary() };
        const auto& textureDirectoryPath{ DLEngine::Texture::GetTextureDirectoryPath() };

        DLEngine::TextureSpecification textureSpecification{};
        textureSpecification.Usage = DLEngine::TextureUsage::Texture;

        // R - right, L - left, U - up
        textureSpecification.DebugName = "Smoke Particle Light Map RLU";
        m_SmokeParticlesRLU = textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "smoke\\smoke_RLU.dds");

        // D - down, B - back, F - front
        textureSpecification.DebugName = "Smoke Particle Light Map DBF";
        m_SmokeParticlesDBF = textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "smoke\\smoke_DBF.dds");

        // E - emission, MV - motion vector, A - alpha
        textureSpecification.DebugName = "Smoke Particle Light Map EMVA";
        m_SmokeParticlesEMVA = textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "smoke\\smoke_MVEA.dds");

        CBTextureAtlasData textureAtlasData{};
        textureAtlasData.Size = Math::Vec2{
            static_cast<float>(m_SmokeParticlesEMVA->GetWidth()),
            static_cast<float>(m_SmokeParticlesEMVA->GetHeight())
        };
        textureAtlasData.TileSize = Math::Vec2{ textureAtlasData.Size.x / 8.0f, textureAtlasData.Size.y / 8.0f };
        textureAtlasData.TilesCountX = 8u;
        textureAtlasData.TilesCountY = 8u;
        m_CBTextureAtlasData->SetData(Buffer{ &textureAtlasData, sizeof(CBTextureAtlasData) });
#endif

        TextureSpecification nullTextureSpec{};
        nullTextureSpec.DebugName = "Null Texture";
        nullTextureSpec.Usage = TextureUsage::TextureAttachment;
        nullTextureSpec.Width = 1u;
        nullTextureSpec.Height = 1u;
        nullTextureSpec.Format = TextureFormat::R8_UNORM;
        m_NullTexture = Texture2D::Create(nullTextureSpec);

        BuildIrradianceMap();
        BuildPrefilteredMap();
    }

    void SceneRenderer::InitBuffers()
    {
        m_CBCamera = ConstantBuffer::Create(sizeof(CBCamera));
        m_CBPBRSettings = ConstantBuffer::Create(sizeof(CBPBRSettings));
        m_CBLightsCount = ConstantBuffer::Create(sizeof(CBLightsCount));
        m_CBPostProcessSettings = ConstantBuffer::Create(sizeof(CBPostProcessingSettings));
        m_SceneShadowEnvironment.CBPointLightData = ConstantBuffer::Create(sizeof(CBOmnidirectionalLightShadowData));
        m_CBShadowMappingData = ConstantBuffer::Create(sizeof(CBShadowMappingData));
        m_CBTextureAtlasData = ConstantBuffer::Create(sizeof(CBTextureAtlasData));

        m_SBDirectionalLights = StructuredBuffer::Create(sizeof(DirectionalLight), 100u);
        m_SBPointLights = StructuredBuffer::Create(sizeof(PointLight), 100u);
        m_SBSpotLights = StructuredBuffer::Create(sizeof(SpotLight), 100u);
        m_SceneShadowEnvironment.SBDirectionalLightsPOVs = StructuredBuffer::Create(sizeof(Math::Mat4x4), 100u);
        m_SceneShadowEnvironment.SBPointLightsPOVs = StructuredBuffer::Create(sizeof(Math::Mat4x4), 100u);
        m_SceneShadowEnvironment.SBSpotLightsPOVs = StructuredBuffer::Create(sizeof(Math::Mat4x4), 100u);
    }

    void SceneRenderer::InitFramebuffers()
    {
        TextureSpecification gBufferAlbedoSpecification{};
        gBufferAlbedoSpecification.DebugName = "G-Buffer Albedo";
        gBufferAlbedoSpecification.Format = TextureFormat::RGBA8_UNORM;
        gBufferAlbedoSpecification.Usage = TextureUsage::TextureAttachment;
        gBufferAlbedoSpecification.Width = m_ViewportWidth;
        gBufferAlbedoSpecification.Height = m_ViewportHeight;
        m_GBufferAlbedo = Texture2D::Create(gBufferAlbedoSpecification);

        TextureSpecification gBufferMetalnessRoughnessSpecification{};
        gBufferMetalnessRoughnessSpecification.DebugName = "G-Buffer Metalness + Roughness";
        gBufferMetalnessRoughnessSpecification.Format = TextureFormat::RG8_UNORM;
        gBufferMetalnessRoughnessSpecification.Usage = TextureUsage::TextureAttachment;
        gBufferMetalnessRoughnessSpecification.Width = m_ViewportWidth;
        gBufferMetalnessRoughnessSpecification.Height = m_ViewportHeight;
        m_GBufferMetalnessRoughness = Texture2D::Create(gBufferMetalnessRoughnessSpecification);

        TextureSpecification gBufferGeometrySurfaceNormalsSpecification{};
        gBufferGeometrySurfaceNormalsSpecification.DebugName = "G-Buffer Geometry + Surface Normals";
        gBufferGeometrySurfaceNormalsSpecification.Format = TextureFormat::RGBA16_SNORM;
        gBufferGeometrySurfaceNormalsSpecification.Usage = TextureUsage::TextureAttachment;
        gBufferGeometrySurfaceNormalsSpecification.Width = m_ViewportWidth;
        gBufferGeometrySurfaceNormalsSpecification.Height = m_ViewportHeight;
        m_GBufferGeometrySurfaceNormals = Texture2D::Create(gBufferGeometrySurfaceNormalsSpecification);

        TextureSpecification gBufferEmissionSpecification{};
        gBufferEmissionSpecification.DebugName = "G-Buffer Emission";
        gBufferEmissionSpecification.Format = TextureFormat::RGBA16_FLOAT;
        gBufferEmissionSpecification.Usage = TextureUsage::TextureAttachment;
        gBufferEmissionSpecification.Width = m_ViewportWidth;
        gBufferEmissionSpecification.Height = m_ViewportHeight;
        m_GBufferEmission = Texture2D::Create(gBufferEmissionSpecification);

        TextureSpecification gBufferDepthSpecification{};
        gBufferDepthSpecification.DebugName = "G-Buffer Depth";
        gBufferDepthSpecification.Format = TextureFormat::DEPTH_R24G8_TYPELESS;
        gBufferDepthSpecification.Usage = TextureUsage::TextureAttachment;
        gBufferDepthSpecification.Width = m_ViewportWidth;
        gBufferDepthSpecification.Height = m_ViewportHeight;
        m_GBufferDepthStencil = Texture2D::Create(gBufferDepthSpecification);

        TextureSpecification hdrResolveTextureSpecification{};
        hdrResolveTextureSpecification.DebugName = "HDR Resolve Texture";
        hdrResolveTextureSpecification.Format = TextureFormat::RGBA16_FLOAT;
        hdrResolveTextureSpecification.Usage = TextureUsage::TextureAttachment;
        hdrResolveTextureSpecification.Width = m_ViewportWidth;
        hdrResolveTextureSpecification.Height = m_ViewportHeight;
        m_HDR_ResolveTexture = Texture2D::Create(hdrResolveTextureSpecification);

        FramebufferSpecification gBufferPBR_StaticFramebufferSpecification{};
        gBufferPBR_StaticFramebufferSpecification.DebugName = "G-Buffer PBR_Static Framebuffer";
        gBufferPBR_StaticFramebufferSpecification.ExistingAttachments = {
            { 0u, m_GBufferAlbedo                 },
            { 1u, m_GBufferMetalnessRoughness     },
            { 2u, m_GBufferGeometrySurfaceNormals },
            { 3u, m_GBufferEmission               },
            { 4u, m_GBufferDepthStencil           }
        };
        gBufferPBR_StaticFramebufferSpecification.Width = m_ViewportWidth;
        gBufferPBR_StaticFramebufferSpecification.Height = m_ViewportHeight;
        gBufferPBR_StaticFramebufferSpecification.StencilReferenceValue = 1u;
        m_GBuffer_PBR_StaticFramebuffer = Framebuffer::Create(gBufferPBR_StaticFramebufferSpecification);

        FramebufferSpecification gBufferEmissionFramebufferSpecification{};
        gBufferEmissionFramebufferSpecification.DebugName = "G-Buffer Emission Framebuffer";
        gBufferEmissionFramebufferSpecification.ExistingAttachments = {
            { 0u, m_GBufferAlbedo                 },
            { 1u, m_GBufferMetalnessRoughness     },
            { 2u, m_GBufferGeometrySurfaceNormals },
            { 3u, m_GBufferEmission               },
            { 4u, m_GBufferDepthStencil           }
        };
        gBufferEmissionFramebufferSpecification.Width = m_ViewportWidth;
        gBufferEmissionFramebufferSpecification.Height = m_ViewportHeight;
        gBufferEmissionFramebufferSpecification.StencilReferenceValue = 2u;
        m_GBuffer_EmissionFramebuffer = Framebuffer::Create(gBufferEmissionFramebufferSpecification);

        FramebufferSpecification hdrResolvePBR_StaticFramebufferSpecification{};
        hdrResolvePBR_StaticFramebufferSpecification.DebugName = "HDR Resolve PBR_Static Framebuffer";
        hdrResolvePBR_StaticFramebufferSpecification.ExistingAttachments = {
            { 0u, m_HDR_ResolveTexture  },
            { 1u, m_GBufferDepthStencil }
        };
        hdrResolvePBR_StaticFramebufferSpecification.Width = m_ViewportWidth;
        hdrResolvePBR_StaticFramebufferSpecification.Height = m_ViewportHeight;
        hdrResolvePBR_StaticFramebufferSpecification.StencilReferenceValue = 1u;
        m_HDR_ResolvePBR_StaticFramebuffer = Framebuffer::Create(hdrResolvePBR_StaticFramebufferSpecification);

        FramebufferSpecification hdrResolveEmissionFramebufferSpecification{};
        hdrResolveEmissionFramebufferSpecification.DebugName = "HDR Resolve Emission Framebuffer";
        hdrResolveEmissionFramebufferSpecification.ExistingAttachments = {
            { 0u, m_HDR_ResolveTexture  },
            { 1u, m_GBufferDepthStencil }
        };
        hdrResolveEmissionFramebufferSpecification.Width = m_ViewportWidth;
        hdrResolveEmissionFramebufferSpecification.Height = m_ViewportHeight;
        hdrResolveEmissionFramebufferSpecification.StencilReferenceValue = 2u;
        m_HDR_ResolveEmissionFramebuffer = Framebuffer::Create(hdrResolveEmissionFramebufferSpecification);

        FramebufferSpecification hdrResolveFramebufferSpecification{};
        hdrResolveFramebufferSpecification.DebugName = "HDR Resolve Framebuffer";
        hdrResolveFramebufferSpecification.ExistingAttachments = {
            { 0u, m_HDR_ResolveTexture  },
            { 1u, m_GBufferDepthStencil }
        };
        hdrResolveFramebufferSpecification.Width = m_ViewportWidth;
        hdrResolveFramebufferSpecification.Height = m_ViewportHeight;
        m_HDR_ResolveFramebuffer = Framebuffer::Create(hdrResolveFramebufferSpecification);

        FramebufferSpecification directionalShadowMapFramebufferSpecification{};
        directionalShadowMapFramebufferSpecification.DebugName = "Directional Shadow Map Framebuffer";
        directionalShadowMapFramebufferSpecification.Attachments = { { TextureFormat::DEPTH_R24G8_TYPELESS, TextureUsage::TextureAttachment } };
        directionalShadowMapFramebufferSpecification.Width = m_SceneShadowEnvironment.Settings.MapSize;
        directionalShadowMapFramebufferSpecification.Height = m_SceneShadowEnvironment.Settings.MapSize;
        m_DirectionalShadowMapFramebuffer = Framebuffer::Create(directionalShadowMapFramebufferSpecification);

        FramebufferSpecification pointShadowMapFramebufferSpecification{};
        pointShadowMapFramebufferSpecification.DebugName = "Point Shadow Map Framebuffer";
        pointShadowMapFramebufferSpecification.Attachments = { { TextureFormat::DEPTH_R24G8_TYPELESS, TextureUsage::TextureAttachment } };
        pointShadowMapFramebufferSpecification.AttachmentsType = TextureType::TextureCube;
        pointShadowMapFramebufferSpecification.Width = m_SceneShadowEnvironment.Settings.MapSize;
        pointShadowMapFramebufferSpecification.Height = m_SceneShadowEnvironment.Settings.MapSize;
        m_PointShadowMapFramebuffer = Framebuffer::Create(pointShadowMapFramebufferSpecification);

        FramebufferSpecification spotShadowMapFramebufferSpecification{};
        spotShadowMapFramebufferSpecification.DebugName = "Spot Shadow Map Framebuffer";
        spotShadowMapFramebufferSpecification.Attachments = { { TextureFormat::DEPTH_R24G8_TYPELESS, TextureUsage::TextureAttachment } };
        spotShadowMapFramebufferSpecification.Width = m_SceneShadowEnvironment.Settings.MapSize;
        spotShadowMapFramebufferSpecification.Height = m_SceneShadowEnvironment.Settings.MapSize;
        m_SpotShadowMapFramebuffer = Framebuffer::Create(spotShadowMapFramebufferSpecification);

        m_SceneShadowEnvironment.DirectionalShadowMaps = AsRef<Texture2D>(m_DirectionalShadowMapFramebuffer->GetDepthAttachment());
        m_SceneShadowEnvironment.PointShadowMaps = AsRef<TextureCube>(m_PointShadowMapFramebuffer->GetDepthAttachment());
        m_SceneShadowEnvironment.SpotShadowMaps = AsRef<Texture2D>(m_SpotShadowMapFramebuffer->GetDepthAttachment());
    }

    void SceneRenderer::InitPipelines()
    {
        PipelineSpecification gBufferPBR_StaticPipelineSpecification{};
        gBufferPBR_StaticPipelineSpecification.DebugName = "G-Buffer PBR_Static Pipeline";
        gBufferPBR_StaticPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("GBuffer_PBR_Static");
        gBufferPBR_StaticPipelineSpecification.TargetFramebuffer = m_GBuffer_PBR_StaticFramebuffer;
        gBufferPBR_StaticPipelineSpecification.DepthStencilState.DepthTest = true;
        gBufferPBR_StaticPipelineSpecification.DepthStencilState.DepthWrite = true;
        gBufferPBR_StaticPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Greater;
        gBufferPBR_StaticPipelineSpecification.DepthStencilState.StencilTest = true;
        gBufferPBR_StaticPipelineSpecification.DepthStencilState.FrontFace.PassOp = StencilOperator::Replace;
        gBufferPBR_StaticPipelineSpecification.DepthStencilState.FrontFace.CompareOp = CompareOperator::Always;
        gBufferPBR_StaticPipelineSpecification.DepthStencilState.BackFace.CompareOp = CompareOperator::Never;
        m_GBuffer_PBR_StaticPipeline = Pipeline::Create(gBufferPBR_StaticPipelineSpecification);

        PipelineSpecification gBufferPBR_Static_DissolutionPipelineSpecification{};
        gBufferPBR_Static_DissolutionPipelineSpecification.DebugName = "G-Buffer PBR_Static Dissolution Pipeline";
        gBufferPBR_Static_DissolutionPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("GBuffer_PBR_Static_Dissolution");
        gBufferPBR_Static_DissolutionPipelineSpecification.TargetFramebuffer = m_GBuffer_PBR_StaticFramebuffer;
        gBufferPBR_Static_DissolutionPipelineSpecification.DepthStencilState.DepthTest = true;
        gBufferPBR_Static_DissolutionPipelineSpecification.DepthStencilState.DepthWrite = true;
        gBufferPBR_Static_DissolutionPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Greater;
        gBufferPBR_Static_DissolutionPipelineSpecification.DepthStencilState.StencilTest = true;
        gBufferPBR_Static_DissolutionPipelineSpecification.DepthStencilState.FrontFace.PassOp = StencilOperator::Replace;
        gBufferPBR_Static_DissolutionPipelineSpecification.DepthStencilState.FrontFace.CompareOp = CompareOperator::Always;
        gBufferPBR_Static_DissolutionPipelineSpecification.DepthStencilState.BackFace.CompareOp = CompareOperator::Never;
        m_GBuffer_PBR_Static_DissolutionPipeline = Pipeline::Create(gBufferPBR_Static_DissolutionPipelineSpecification);

        PipelineSpecification gBufferEmissionPipelineSpecification{};
        gBufferEmissionPipelineSpecification.DebugName = "G-Buffer Emission Pipeline";
        gBufferEmissionPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("GBuffer_Emission");
        gBufferEmissionPipelineSpecification.TargetFramebuffer = m_GBuffer_EmissionFramebuffer;
        gBufferEmissionPipelineSpecification.DepthStencilState.DepthTest = true;
        gBufferEmissionPipelineSpecification.DepthStencilState.DepthWrite = true;
        gBufferEmissionPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Greater;
        gBufferEmissionPipelineSpecification.DepthStencilState.StencilTest = true;
        gBufferEmissionPipelineSpecification.DepthStencilState.FrontFace.PassOp = StencilOperator::Replace;
        gBufferEmissionPipelineSpecification.DepthStencilState.FrontFace.CompareOp = CompareOperator::Always;
        gBufferEmissionPipelineSpecification.DepthStencilState.BackFace.CompareOp = CompareOperator::Never;
        m_GBuffer_EmissionPipeline = Pipeline::Create(gBufferEmissionPipelineSpecification);

        PipelineSpecification gBufferResolve_PBR_StaticPipelineSpecification{};
        gBufferResolve_PBR_StaticPipelineSpecification.DebugName = "G-Buffer Resolve PBR_Static Pipeline";
        gBufferResolve_PBR_StaticPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("GBufferResolve_PBR_Static");
        gBufferResolve_PBR_StaticPipelineSpecification.TargetFramebuffer = m_HDR_ResolvePBR_StaticFramebuffer;
        gBufferResolve_PBR_StaticPipelineSpecification.DepthStencilState.DepthTest = false;
        gBufferResolve_PBR_StaticPipelineSpecification.DepthStencilState.DepthWrite = false;
        gBufferResolve_PBR_StaticPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Always;
        gBufferResolve_PBR_StaticPipelineSpecification.DepthStencilState.StencilTest = true;
        gBufferResolve_PBR_StaticPipelineSpecification.DepthStencilState.FrontFace.CompareOp = CompareOperator::Equal;
        gBufferResolve_PBR_StaticPipelineSpecification.DepthStencilState.BackFace.CompareOp = CompareOperator::Never;
        m_GBufferResolve_PBR_StaticPipeline = Pipeline::Create(gBufferResolve_PBR_StaticPipelineSpecification);

        PipelineSpecification gBufferResolve_EmissionPipelineSpecification{};
        gBufferResolve_EmissionPipelineSpecification.DebugName = "G-Buffer Resolve Emission Pipeline";
        gBufferResolve_EmissionPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("GBufferResolve_Emission");
        gBufferResolve_EmissionPipelineSpecification.TargetFramebuffer = m_HDR_ResolveEmissionFramebuffer;
        gBufferResolve_EmissionPipelineSpecification.DepthStencilState.DepthTest = false;
        gBufferResolve_EmissionPipelineSpecification.DepthStencilState.DepthWrite = false;
        gBufferResolve_EmissionPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Always;
        gBufferResolve_EmissionPipelineSpecification.DepthStencilState.StencilTest = true;
        gBufferResolve_EmissionPipelineSpecification.DepthStencilState.FrontFace.CompareOp = CompareOperator::Equal;
        gBufferResolve_EmissionPipelineSpecification.DepthStencilState.BackFace.CompareOp = CompareOperator::Never;
        m_GBufferResolve_EmissionPipeline = Pipeline::Create(gBufferResolve_EmissionPipelineSpecification);

        PipelineSpecification skyboxPipelineSpecification{};
        skyboxPipelineSpecification.DebugName = "Skybox Pipeline";
        skyboxPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("Skybox");
        skyboxPipelineSpecification.TargetFramebuffer = m_HDR_ResolveFramebuffer;
        skyboxPipelineSpecification.DepthStencilState.DepthTest = true;
        skyboxPipelineSpecification.DepthStencilState.DepthWrite = false;
        skyboxPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::GreaterOrEqual;
        m_SkyboxPipeline = Pipeline::Create(skyboxPipelineSpecification);

        PipelineSpecification postProcessPipelineSpecification{};
        postProcessPipelineSpecification.DebugName = "Post Process Pipeline";
        postProcessPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("PostProcess");
        postProcessPipelineSpecification.TargetFramebuffer = Renderer::GetSwapChainTargetFramebuffer();
        postProcessPipelineSpecification.DepthStencilState.DepthTest = false;
        postProcessPipelineSpecification.DepthStencilState.DepthWrite = false;
        postProcessPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Always;
        m_PostProcessPipeline = Pipeline::Create(postProcessPipelineSpecification);

        PipelineSpecification directionalShadowMapPipelineSpecification{};
        directionalShadowMapPipelineSpecification.DebugName = "Directional Shadow Map Pipeline";
        directionalShadowMapPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("ShadowMap_Directional");
        directionalShadowMapPipelineSpecification.TargetFramebuffer = m_DirectionalShadowMapFramebuffer;
        directionalShadowMapPipelineSpecification.DepthStencilState.DepthTest = true;
        directionalShadowMapPipelineSpecification.DepthStencilState.DepthWrite = true;
        directionalShadowMapPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Greater;
        m_DirectionalShadowMapPipeline = Pipeline::Create(directionalShadowMapPipelineSpecification);

        PipelineSpecification pointShadowMapPipelineSpecification{};
        pointShadowMapPipelineSpecification.DebugName = "Point Shadow Map Pipeline";
        pointShadowMapPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("ShadowMap_Omnidirectional");
        pointShadowMapPipelineSpecification.TargetFramebuffer = m_PointShadowMapFramebuffer;
        pointShadowMapPipelineSpecification.DepthStencilState.DepthTest = true;
        pointShadowMapPipelineSpecification.DepthStencilState.DepthWrite = true;
        pointShadowMapPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Greater;
        m_PointShadowMapPipeline = Pipeline::Create(pointShadowMapPipelineSpecification);

        PipelineSpecification spotShadowMapPipelineSpecification{};
        spotShadowMapPipelineSpecification.DebugName = "Spot Shadow Map Pipeline";
        spotShadowMapPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("ShadowMap_Directional");
        spotShadowMapPipelineSpecification.TargetFramebuffer = m_SpotShadowMapFramebuffer;
        spotShadowMapPipelineSpecification.DepthStencilState.DepthTest = true;
        spotShadowMapPipelineSpecification.DepthStencilState.DepthWrite = true;
        spotShadowMapPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Greater;
        m_SpotShadowMapPipeline = Pipeline::Create(spotShadowMapPipelineSpecification);
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
        UpdateSmokeParticlesData();

        // Resizing 
        m_GBuffer_PBR_StaticFramebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
        m_GBuffer_EmissionFramebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
        m_HDR_ResolvePBR_StaticFramebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
        m_HDR_ResolveEmissionFramebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
        m_HDR_ResolveFramebuffer->Resize(m_ViewportWidth, m_ViewportHeight);

        if (m_DirectionalShadowMapFramebuffer->GetDepthAttachment()->GetLayersCount() < lightsCount.DirectionalLightsCount)
        {
            FramebufferSpecification directionalShadowMapFBSpec{};
            directionalShadowMapFBSpec.DebugName = "Directional Shadow Map Framebuffer";
            directionalShadowMapFBSpec.Attachments = {
                { TextureFormat::DEPTH_R24G8_TYPELESS, TextureUsage::TextureAttachment, 1u, lightsCount.DirectionalLightsCount }
            };
            directionalShadowMapFBSpec.Width = m_SceneShadowEnvironment.Settings.MapSize;
            directionalShadowMapFBSpec.Height = m_SceneShadowEnvironment.Settings.MapSize;
            m_DirectionalShadowMapFramebuffer = Framebuffer::Create(directionalShadowMapFBSpec);

            m_SceneShadowEnvironment.DirectionalShadowMaps = AsRef<Texture2D>(m_DirectionalShadowMapFramebuffer->GetDepthAttachment());

            PipelineSpecification directionalShadowMapPipelineSpecification{};
            directionalShadowMapPipelineSpecification.DebugName = "Directional Shadow Map Pipeline";
            directionalShadowMapPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("ShadowMap_Directional");
            directionalShadowMapPipelineSpecification.TargetFramebuffer = m_DirectionalShadowMapFramebuffer;
            directionalShadowMapPipelineSpecification.DepthStencilState.DepthTest = true;
            directionalShadowMapPipelineSpecification.DepthStencilState.DepthWrite = true;
            directionalShadowMapPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Greater;
            m_DirectionalShadowMapPipeline = Pipeline::Create(directionalShadowMapPipelineSpecification);
        }

        if (m_PointShadowMapFramebuffer->GetDepthAttachment()->GetLayersCount() < lightsCount.PointLightsCount)
        {
            FramebufferSpecification pointShadowMapFBSpec{};
            pointShadowMapFBSpec.DebugName = "Point Shadow Map Framebuffer";
            pointShadowMapFBSpec.Attachments = {
                { TextureFormat::DEPTH_R24G8_TYPELESS, TextureUsage::TextureAttachment, 1u, lightsCount.PointLightsCount }
            };
            pointShadowMapFBSpec.AttachmentsType = TextureType::TextureCube;
            pointShadowMapFBSpec.Width = m_SceneShadowEnvironment.Settings.MapSize;
            pointShadowMapFBSpec.Height = m_SceneShadowEnvironment.Settings.MapSize;
            m_PointShadowMapFramebuffer = Framebuffer::Create(pointShadowMapFBSpec);

            m_SceneShadowEnvironment.PointShadowMaps = AsRef<TextureCube>(m_PointShadowMapFramebuffer->GetDepthAttachment());

            PipelineSpecification pointShadowMapPipelineSpecification{};
            pointShadowMapPipelineSpecification.DebugName = "Point Shadow Map Pipeline";
            pointShadowMapPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("ShadowMap_Omnidirectional");
            pointShadowMapPipelineSpecification.TargetFramebuffer = m_PointShadowMapFramebuffer;
            pointShadowMapPipelineSpecification.DepthStencilState.DepthTest = true;
            pointShadowMapPipelineSpecification.DepthStencilState.DepthWrite = true;
            pointShadowMapPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Greater;
            m_PointShadowMapPipeline = Pipeline::Create(pointShadowMapPipelineSpecification);
        }

        if (m_SpotShadowMapFramebuffer->GetDepthAttachment()->GetLayersCount() < lightsCount.SpotLightsCount)
        {
            FramebufferSpecification spotShadowMapFBSpec{};
            spotShadowMapFBSpec.DebugName = "Spot Shadow Map Framebuffer";
            spotShadowMapFBSpec.Attachments = {
                { TextureFormat::DEPTH_R24G8_TYPELESS, TextureUsage::TextureAttachment, 1u, lightsCount.SpotLightsCount }
            };
            spotShadowMapFBSpec.Width = m_SceneShadowEnvironment.Settings.MapSize;
            spotShadowMapFBSpec.Height = m_SceneShadowEnvironment.Settings.MapSize;
            m_SpotShadowMapFramebuffer = Framebuffer::Create(spotShadowMapFBSpec);

            m_SceneShadowEnvironment.SpotShadowMaps = AsRef<Texture2D>(m_SpotShadowMapFramebuffer->GetDepthAttachment());

            PipelineSpecification spotShadowMapPipelineSpecification{};
            spotShadowMapPipelineSpecification.DebugName = "Spot Shadow Map Pipeline";
            spotShadowMapPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("ShadowMap_Directional");
            spotShadowMapPipelineSpecification.TargetFramebuffer = m_SpotShadowMapFramebuffer;
            spotShadowMapPipelineSpecification.DepthStencilState.DepthTest = true;
            spotShadowMapPipelineSpecification.DepthStencilState.DepthWrite = true;
            spotShadowMapPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Greater;
            m_SpotShadowMapPipeline = Pipeline::Create(spotShadowMapPipelineSpecification);
        }

        m_DirectionalShadowMapFramebuffer->Resize(m_SceneShadowEnvironment.Settings.MapSize, m_SceneShadowEnvironment.Settings.MapSize);
        m_PointShadowMapFramebuffer->Resize(m_SceneShadowEnvironment.Settings.MapSize, m_SceneShadowEnvironment.Settings.MapSize);
        m_SpotShadowMapFramebuffer->Resize(m_SceneShadowEnvironment.Settings.MapSize, m_SceneShadowEnvironment.Settings.MapSize);
    }

    void SceneRenderer::ShadowPass()
    {
        const auto& lightsCount{ m_CBLightsCount->GetLocalData().As<CBLightsCount>() };

        // Building directional shadow maps
        for (uint32_t i{ 0u }; i < lightsCount->DirectionalLightsCount; ++i)
        {
            const auto& directionalLightData{ m_SceneShadowEnvironment.DirectionalLightsData[i] };

            UpdateCBCamera(directionalLightData.POV);

            TextureViewSpecification depthAttachmentViewSpec{};
            depthAttachmentViewSpec.Format = TextureFormat::DEPTH24STENCIL8;
            depthAttachmentViewSpec.Subresource.BaseMip = 0u;
            depthAttachmentViewSpec.Subresource.MipsCount = 1u;
            depthAttachmentViewSpec.Subresource.BaseLayer = i;
            depthAttachmentViewSpec.Subresource.LayersCount = 1u;
            m_DirectionalShadowMapFramebuffer->SetDepthAttachmentViewSpecification(depthAttachmentViewSpec);

            Renderer::SetPipeline(m_DirectionalShadowMapPipeline, DL_CLEAR_DEPTH_ATTACHMENT);
            Utils::SubmitMeshBatch(m_Scene->m_MeshRegistry, "GBuffer_PBR_Static", false);
            Utils::SubmitMeshBatch(m_Scene->m_MeshRegistry, "GBuffer_PBR_Static_Dissolution", false);
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
            m_SceneShadowEnvironment.CBPointLightData->SetData(Buffer{ &pointLightShadowData, sizeof(CBOmnidirectionalLightShadowData) });

            TextureViewSpecification depthAttachmentViewSpec{};
            depthAttachmentViewSpec.Format = TextureFormat::DEPTH24STENCIL8;
            depthAttachmentViewSpec.Subresource.BaseMip = 0u;
            depthAttachmentViewSpec.Subresource.MipsCount = 1u;
            depthAttachmentViewSpec.Subresource.BaseLayer = i;
            depthAttachmentViewSpec.Subresource.LayersCount = 1u;
            m_PointShadowMapFramebuffer->SetDepthAttachmentViewSpecification(depthAttachmentViewSpec);

            Renderer::SetPipeline(m_PointShadowMapPipeline, DL_CLEAR_DEPTH_ATTACHMENT);
            Utils::SubmitMeshBatch(m_Scene->m_MeshRegistry, "GBuffer_PBR_Static", false);
            Utils::SubmitMeshBatch(m_Scene->m_MeshRegistry, "GBuffer_PBR_Static_Dissolution", false);
        }

        // Building spot shadow maps
        for (uint32_t i{ 0u }; i < lightsCount->SpotLightsCount; ++i)
        {
            const auto& spotLightData{ m_SceneShadowEnvironment.SpotLightsData[i] };

            UpdateCBCamera(spotLightData.POV);

            TextureViewSpecification depthAttachmentViewSpec{};
            depthAttachmentViewSpec.Format = TextureFormat::DEPTH24STENCIL8;
            depthAttachmentViewSpec.Subresource.BaseMip = 0u;
            depthAttachmentViewSpec.Subresource.MipsCount = 1u;
            depthAttachmentViewSpec.Subresource.BaseLayer = i;
            depthAttachmentViewSpec.Subresource.LayersCount = 1u;
            m_SpotShadowMapFramebuffer->SetDepthAttachmentViewSpecification(depthAttachmentViewSpec);

            Renderer::SetPipeline(m_SpotShadowMapPipeline, DL_CLEAR_DEPTH_ATTACHMENT);
            Utils::SubmitMeshBatch(m_Scene->m_MeshRegistry, "GBuffer_PBR_Static", false);
            Utils::SubmitMeshBatch(m_Scene->m_MeshRegistry, "GBuffer_PBR_Static_Dissolution", false);
        }
    }

    void SceneRenderer::GBufferPass()
    {
        UpdateCBCamera(m_Scene->m_SceneCameraController.GetCamera());

        TextureViewSpecification depthAttachmentWriteSpecification{};
        depthAttachmentWriteSpecification.Format = TextureFormat::DEPTH24STENCIL8;

        m_GBuffer_EmissionFramebuffer->SetDepthAttachmentViewSpecification(depthAttachmentWriteSpecification);
        Renderer::SetPipeline(m_GBuffer_EmissionPipeline, DL_CLEAR_COLOR_ATTACHMENT | DL_CLEAR_DEPTH_ATTACHMENT | DL_CLEAR_STENCIL_ATTACHMENT);
        Utils::SubmitMeshBatch(m_Scene->m_MeshRegistry, "GBuffer_Emission", true);

        m_GBuffer_PBR_StaticFramebuffer->SetDepthAttachmentViewSpecification(depthAttachmentWriteSpecification);
        Renderer::SetPipeline(m_GBuffer_PBR_StaticPipeline, DL_CLEAR_NONE);
        Utils::SubmitMeshBatch(m_Scene->m_MeshRegistry, "GBuffer_PBR_Static", true);

        Renderer::SetPipeline(m_GBuffer_PBR_Static_DissolutionPipeline, DL_CLEAR_NONE);
        Utils::SubmitMeshBatch(m_Scene->m_MeshRegistry, "GBuffer_PBR_Static_Dissolution", true);
    }

    void SceneRenderer::FullscreenPass()
    {
        m_GBufferDepthStencilCopy = Texture2D::Copy(m_GBufferDepthStencil);

        const auto& lightsCount{ m_CBLightsCount->GetLocalData().As<CBLightsCount>() };

        TextureViewSpecification directionalShadowMaps{};
        directionalShadowMaps.Format = TextureFormat::R24_UNORM_X8_TYPELESS;
        directionalShadowMaps.Subresource.BaseMip = 0u;
        directionalShadowMaps.Subresource.MipsCount = 1u;
        directionalShadowMaps.Subresource.BaseLayer = 0u;
        directionalShadowMaps.Subresource.LayersCount = lightsCount->DirectionalLightsCount;
        Renderer::SetTexture2Ds(BP_TEX_DIRECTIONAL_SHADOW_MAPS, DL_PIXEL_SHADER_BIT, { m_SceneShadowEnvironment.DirectionalShadowMaps }, { directionalShadowMaps });

        TextureViewSpecification pointShadowMaps{};
        pointShadowMaps.Format = TextureFormat::R24_UNORM_X8_TYPELESS;
        pointShadowMaps.Subresource.BaseMip = 0u;
        pointShadowMaps.Subresource.MipsCount = 1u;
        pointShadowMaps.Subresource.BaseLayer = 0u;
        pointShadowMaps.Subresource.LayersCount = lightsCount->PointLightsCount;
        Renderer::SetTextureCubes(BP_TEX_POINT_SHADOW_MAPS, DL_PIXEL_SHADER_BIT, { m_SceneShadowEnvironment.PointShadowMaps }, { pointShadowMaps });

        TextureViewSpecification spotShadowMaps{};
        spotShadowMaps.Format = TextureFormat::R24_UNORM_X8_TYPELESS;
        spotShadowMaps.Subresource.BaseMip = 0u;
        spotShadowMaps.Subresource.MipsCount = 1u;
        spotShadowMaps.Subresource.BaseLayer = 0u;
        spotShadowMaps.Subresource.LayersCount = lightsCount->SpotLightsCount;
        Renderer::SetTexture2Ds(BP_TEX_SPOT_SHADOW_MAPS, DL_PIXEL_SHADER_BIT, { m_SceneShadowEnvironment.SpotShadowMaps }, { spotShadowMaps });

        TextureViewSpecification defaultTextureViewSpecification{};

        TextureViewSpecification depthAttachmentViewReadSpecification{};
        depthAttachmentViewReadSpecification.Format = TextureFormat::R24_UNORM_X8_TYPELESS;

        TextureViewSpecification depthAttachmentViewWriteSpecification{};
        depthAttachmentViewWriteSpecification.Format = TextureFormat::DEPTH24STENCIL8;

        m_HDR_ResolvePBR_StaticFramebuffer->SetDepthAttachmentViewSpecification(depthAttachmentViewWriteSpecification);
        Renderer::SetPipeline(m_GBufferResolve_PBR_StaticPipeline, DL_CLEAR_COLOR_ATTACHMENT);
        Renderer::SetTexture2Ds(BP_TEX_GBUFFER_ALBEDO, DL_PIXEL_SHADER_BIT,
            {
                m_GBufferAlbedo,
                m_GBufferMetalnessRoughness,
                m_GBufferGeometrySurfaceNormals,
                m_GBufferEmission,
                m_GBufferDepthStencilCopy
            },
            {
                defaultTextureViewSpecification,
                defaultTextureViewSpecification,
                defaultTextureViewSpecification,
                defaultTextureViewSpecification,
                depthAttachmentViewReadSpecification
            }
        );
        Renderer::SubmitFullscreenQuad();

        m_HDR_ResolveEmissionFramebuffer->SetDepthAttachmentViewSpecification(depthAttachmentViewWriteSpecification);
        Renderer::SetPipeline(m_GBufferResolve_EmissionPipeline, DL_CLEAR_NONE);
        Renderer::SubmitFullscreenQuad();
    }

    void SceneRenderer::SkyboxPass()
    {
        TextureViewSpecification defaultTextureViewSpecification{};
        
        TextureViewSpecification depthAttachmentViewWriteSpecification{};
        depthAttachmentViewWriteSpecification.Format = TextureFormat::DEPTH24STENCIL8;

        m_HDR_ResolveFramebuffer->SetDepthAttachmentViewSpecification(depthAttachmentViewWriteSpecification);
        Renderer::SetPipeline(m_SkyboxPipeline, DL_CLEAR_NONE);
        Renderer::SetTextureCubes(BP_TEX_NEXT_FREE, DL_PIXEL_SHADER_BIT, { m_SceneEnvironment.Skybox }, { defaultTextureViewSpecification });
        Renderer::SubmitFullscreenQuad();
    }

    void SceneRenderer::SmokeParticlesPass()
    {
        if (m_Scene->m_SmokeEnvironment.SortedSmokeParticles.empty())
            return;

#if 0
        TextureViewSpecification depthWriteAttachmentViewSpec{};
        depthWriteAttachmentViewSpec.Format = TextureFormat::DEPTH24STENCIL8;

        TextureViewSpecification depthReadAttachmentViewSpec{};
        depthReadAttachmentViewSpec.Format = TextureFormat::R24_UNORM_X8_TYPELESS;

        // Building max depth framebuffer
        m_MaxDepthFramebuffer->SetDepthAttachmentViewSpecification(depthWriteAttachmentViewSpec);
        Renderer::SetPipeline(m_MaxDepthPipeline, true);

        Renderer::SetConstantBuffers(BP_CB_NEXT_FREE, DL_PIXEL_SHADER_BIT, { m_CBPostProcessSettings });
        
        Renderer::SetTexture2Ds(BP_TEX_NEXT_FREE, DL_PIXEL_SHADER_BIT, { AsRef<Texture2D>(m_HDRFramebuffer->GetDepthAttachment()) }, { depthReadAttachmentViewSpec });

        Renderer::SubmitFullscreenQuad();

        Renderer::SetTexture2Ds(BP_TEX_NEXT_FREE, DL_PIXEL_SHADER_BIT, { m_NullTexture }, { TextureViewSpecification{} });

        // Rendering smoke particles
        Renderer::SetPipeline(m_SmokeParticlePipeline, false);
        Renderer::SetConstantBuffers(BP_CB_NEXT_FREE, DL_VERTEX_SHADER_BIT | DL_PIXEL_SHADER_BIT, { m_CBTextureAtlasData });
        Renderer::SetTexture2Ds(BP_TEX_NEXT_FREE, DL_PIXEL_SHADER_BIT,
            {
                m_SmokeParticlesRLU,
                m_SmokeParticlesDBF,
                m_SmokeParticlesEMVA,
                AsRef<Texture2D>(m_MaxDepthFramebuffer->GetDepthAttachment())
            },
            {
                TextureViewSpecification{},
                TextureViewSpecification{},
                TextureViewSpecification{},
                depthReadAttachmentViewSpec
            }
        );
        
        Renderer::SubmitParticleBillboard(m_SmokeParticlesInstanceBuffer);
#endif
    }

    void SceneRenderer::PostProcessPass()
    {
        Renderer::SetPipeline(m_PostProcessPipeline, DL_CLEAR_COLOR_ATTACHMENT);
        Renderer::SetConstantBuffers(BP_CB_NEXT_FREE, DL_PIXEL_SHADER_BIT, { m_CBPostProcessSettings });
        Renderer::SetTexture2Ds(BP_TEX_NEXT_FREE, DL_PIXEL_SHADER_BIT, { m_HDR_ResolveTexture }, { TextureViewSpecification{} });
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

        cameraData.zNear = camera.GetNearZ();
        cameraData.zFar  = camera.GetFarZ();
        
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
                constexpr std::array<Math::Vec3, 6u> cubeDirections{
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
            }
        }
        m_SceneShadowEnvironment.SBSpotLightsPOVs->Unmap();
        m_SBSpotLights->Unmap();
    }

    void SceneRenderer::UpdateSmokeParticlesData()
    {
        const uint32_t smokeParticlesCount{ static_cast<uint32_t>(m_Scene->m_SmokeEnvironment.SortedSmokeParticles.size()) };
        
        if (smokeParticlesCount == 0u)
            return;

        // Recreate smoke particles instance buffer if needed
        const auto& instanceBufferLayout{ m_SmokeParticlesInstanceBuffer->GetLayout() };
        const size_t requiredSmokeParticlesInstanceBufferSize{ instanceBufferLayout.GetStride() * smokeParticlesCount };
        if (m_SmokeParticlesInstanceBuffer->GetSize() != requiredSmokeParticlesInstanceBufferSize)
            m_SmokeParticlesInstanceBuffer = VertexBuffer::Create(instanceBufferLayout, requiredSmokeParticlesInstanceBufferSize);

        auto* smokeParticlesInstanceBuffer{ m_SmokeParticlesInstanceBuffer->Map().As<VBSmokeParticle>() };
        for (uint32_t sortedParticleIndex{ 0u }; sortedParticleIndex < smokeParticlesCount; ++sortedParticleIndex)
        {
            const auto& smokeParticleID{ m_Scene->m_SmokeEnvironment.SortedSmokeParticles[sortedParticleIndex] };
            const uint32_t smokeEmitterIndex{ smokeParticleID.first };
            const uint32_t smokeParticleIndex{ smokeParticleID.second };

            const auto& smokeEmitter{ m_Scene->m_SmokeEnvironment.SmokeEmitters[smokeEmitterIndex].first };
            const auto& smokeParticle{ smokeEmitter.Particles[smokeParticleIndex] };

            VBSmokeParticle gpuSmokeParticle{};
            gpuSmokeParticle.WorldPosition = smokeParticle.Position;
            gpuSmokeParticle.TintColor = smokeEmitter.SpawnedParticleTintColor;
            gpuSmokeParticle.InitialSize = smokeEmitter.InitialParticleSize;
            gpuSmokeParticle.EndSize = smokeEmitter.FinalParticleSize;
            gpuSmokeParticle.EmissionIntensity = smokeEmitter.ParticleEmissionIntensity;
            gpuSmokeParticle.LifetimeMS = smokeParticle.LifetimeMS;
            gpuSmokeParticle.LifetimePassedMS = smokeParticle.LifetimePassedMS;
            gpuSmokeParticle.Rotation = smokeParticle.Rotation;

            smokeParticlesInstanceBuffer[sortedParticleIndex] = gpuSmokeParticle;
        }
        m_SmokeParticlesInstanceBuffer->Unmap();
    }

    void SceneRenderer::BuildIrradianceMap()
    {
        const uint32_t brdfLUTSize{ Renderer::GetBRDFLUT()->GetWidth() };

        TextureSpecification irradianceMapSpec{};
        irradianceMapSpec.DebugName = "Irradiance Map";
        irradianceMapSpec.Format = TextureFormat::RGBA16_FLOAT;
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
        irradianceMapPipelineSpec.DepthStencilState.DepthCompareOp = CompareOperator::Always;
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

        Renderer::SetPipeline(irradianceMapPipeline, DL_CLEAR_COLOR_ATTACHMENT);
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
        prefilteredMapSpec.Format = TextureFormat::RGBA16_FLOAT;
        prefilteredMapSpec.Usage = TextureUsage::TextureAttachment;
        prefilteredMapSpec.Width = brdfLUTSize;
        prefilteredMapSpec.Height = brdfLUTSize;
        prefilteredMapSpec.Mips = static_cast<uint32_t>(Math::Log2(static_cast<float>(brdfLUTSize))) + 1u;
        prefilteredMapSpec.Layers = 1u;
        m_SceneEnvironment.PrefilteredMap = TextureCube::Create(prefilteredMapSpec);

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

        FramebufferSpecification prefilteredMapFramebufferSpecification{};
        prefilteredMapFramebufferSpecification.ExistingAttachments[0] = m_SceneEnvironment.PrefilteredMap;
        prefilteredMapFramebufferSpecification.ClearColor = Math::Vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
        prefilteredMapFramebufferSpecification.AttachmentsType = TextureType::TextureCube;

        PipelineSpecification prefilteredMapPipelineSpecification{};
        prefilteredMapPipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("PrefilteredEnvironment");
        prefilteredMapPipelineSpecification.DepthStencilState.DepthCompareOp = CompareOperator::Always;
        prefilteredMapPipelineSpecification.DepthStencilState.DepthTest = false;
        prefilteredMapPipelineSpecification.DepthStencilState.DepthWrite = false;

        const float roughnessStep{ 1.0f / static_cast<float>(prefilteredMapSpec.Mips - 1u) };
        for (uint32_t mip{ 0u }; mip < prefilteredMapSpec.Mips; ++mip)
        {
            prefilteredMapFramebufferSpecification.DebugName = "Prefiltered Map Framebuffer Mip " + mip;
            prefilteredMapFramebufferSpecification.Width = brdfLUTSize >> mip;
            prefilteredMapFramebufferSpecification.Height = brdfLUTSize >> mip;
            Ref<Framebuffer> prefilteredMapFramebuffer{ Framebuffer::Create(prefilteredMapFramebufferSpecification) };

            TextureViewSpecification framebufferViewSpec{};
            framebufferViewSpec.Subresource.BaseMip = mip;
            framebufferViewSpec.Subresource.MipsCount = 1u; // Actually ignored in D3D11TextureCube
            framebufferViewSpec.Subresource.BaseLayer = 0u;
            framebufferViewSpec.Subresource.LayersCount = 1u;
            prefilteredMapFramebuffer->SetColorAttachmentViewSpecification(0u, framebufferViewSpec);

            prefilteredMapPipelineSpecification.DebugName = "Prefiltered Map Pipeline Mip " + mip;
            prefilteredMapPipelineSpecification.TargetFramebuffer = prefilteredMapFramebuffer;
            Ref<Pipeline> prefilteredMapPipeline{ Pipeline::Create(prefilteredMapPipelineSpecification) };

            CBEnvironmentMapping.Roughness = static_cast<float>(mip) * roughnessStep;
            cbEnvironmentMapping->SetData(Buffer{ &CBEnvironmentMapping, sizeof(CBEnvironmentMapping) });

            Renderer::SetPipeline(prefilteredMapPipeline, DL_CLEAR_COLOR_ATTACHMENT);
            Renderer::SubmitFullscreenQuad();
        }
    }

}