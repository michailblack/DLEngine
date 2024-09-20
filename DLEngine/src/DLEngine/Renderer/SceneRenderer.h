#pragma once
#include "DLEngine/Renderer/Pipeline.h"
#include "DLEngine/Renderer/PipelineCompute.h"
#include "DLEngine/Renderer/Scene.h"
#include "DLEngine/Renderer/StructuredBuffer.h"

namespace DLEngine
{
    struct CBPBRMaterial
    {
        uint32_t UseNormalMap{ static_cast<uint32_t>(true) };
        uint32_t FlipNormalMapY{ static_cast<uint32_t>(false) };
        uint32_t HasMetalnessMap{ static_cast<uint32_t>(true) };
        float DefaultMetalness{ 1.0f };
        uint32_t HasRoughnessMap{ static_cast<uint32_t>(true) };
        float DefaultRoughness{ 0.0f };
        float _padding[2u];
    };

    struct PBRSettings
    {
        Math::Vec3 IndirectLightRadiance{ 0.1f };
        float OverwrittenRoughness{ 0.0f };
        bool OverwriteRoughness{ false };
        bool UseIBL{ true };
        bool UseDiffuseReflections{ true };
        bool UseSpecularReflections{ true };
    };

    struct PostProcessingSettings
    {
        float EV100{ 0.0f };
        float Gamma{ 2.2f };
        float FXAA_QualitySubpix{ 0.75f };
        float FXAA_QualityEdgeThreshold{ 0.063f };
        float FXAA_QualityEdgeThresholdMin{ 0.0312f };
    };

    struct ShadowMappingSettings
    {
        uint32_t MapSize{ 2048u };
        float ShadowBias{ 0.05f };
        float DirectionalLightShadowMargin{ 5.0f };
        float DirectionalLightShadowDistance{ 10.0f };
        bool UseDirectionalShadows{ true };
        bool UseOmnidirectionalShadows{ true };
        bool UseSpotShadows{ true };
        bool UsePCF{ true };
    };

    struct SceneEnvironment
    {
        Ref<TextureCube> Skybox;
        Ref<TextureCube> IrradianceMap;
        Ref<TextureCube> PrefilteredMap;
    };

    struct DirectionalLightShadowMapData
    {
        Camera POV;
        Math::Vec3 PrevSceneCameraPosition;
        float WorldTexelSize{ Math::Numeric::Inf };
    };

    struct PointLightShadowMapData
    {
        std::array<Camera, 6u> POVs;
    };

    struct SpotLightShadowMapData
    {
        Camera POV;
    };

    struct SceneShadowEnvironment
    {
        std::vector<DirectionalLightShadowMapData> DirectionalLightsData;
        std::vector<PointLightShadowMapData> PointLightsData;
        std::vector<SpotLightShadowMapData> SpotLightsData;
      
        Ref<StructuredBuffer> SBDirectionalLightsPOVs;
        Ref<Texture2D> DirectionalShadowMaps;

        Ref<StructuredBuffer> SBPointLightsPOVs;
        Ref<TextureCube> PointShadowMaps;
        Ref<ConstantBuffer> CBPointLightData;

        Ref<StructuredBuffer> SBSpotLightsPOVs;
        Ref<Texture2D> SpotShadowMaps;

        ShadowMappingSettings Settings;

        bool ForceRecreateDirectionalLightShadowMaps{ false };
    };

    struct SceneRendererSpecification
    {
        Ref<TextureCube> Skybox;
        uint32_t ViewportWidth{ 0u };
        uint32_t ViewportHeight{ 0u };
    };

    class SceneRenderer
    {
    public:
        SceneRenderer(const SceneRendererSpecification& specification);

        void RenderScene(const Ref<Scene>& scene);

        void SetPBRSettings(const PBRSettings& pbrSettings);
        void SetPostProcessingSettings(const PostProcessingSettings& postProcessingSettings);
        void SetShadowMappingSettings(const ShadowMappingSettings& shadowMapSettings);

    private:
        void Init();

        void InitBuffers();
        void InitTextures();
        void InitFramebuffers();
        void InitPipelines();

        void PreRender();

        void ShadowPass();
        void GBufferPass();
        void FullscreenPass();
        void SkyboxPass();
        void IncinerationParticlesPass();
        void SmokeParticlesPass();
        void PostProcessPass();

        void UpdateCBCamera(const Camera& camera);
        void UpdateDirectionalLightsData();
        void UpdatePointLightsData();
        void UpdateSpotLightsData();

        void UpdateDecalsData();
        void UpdateSmokeParticlesData();

        void BuildIrradianceMap();
        void BuildPrefilteredMap();

    private:
        SceneShadowEnvironment m_SceneShadowEnvironment;

        SceneEnvironment m_SceneEnvironment;
        
        Ref<Scene> m_Scene;

        Ref<ConstantBuffer> m_CBSceneData;
        Ref<ConstantBuffer> m_CBCamera;
        Ref<ConstantBuffer> m_CBPBRSettings;
        Ref<ConstantBuffer> m_CBShadowMappingData;
        Ref<ConstantBuffer> m_CBLightsCount;
        Ref<ConstantBuffer> m_CBPostProcessSettings;
        Ref<ConstantBuffer> m_CBTextureAtlasData;

        Ref<StructuredBuffer> m_SBDirectionalLights;
        Ref<StructuredBuffer> m_SBPointLights;
        Ref<StructuredBuffer> m_SBSpotLights;

        Ref<Texture2D> m_GBufferAlbedo;
        Ref<Texture2D> m_GBufferMetalnessRoughness;
        Ref<Texture2D> m_GBufferGeometrySurfaceNormals;
        Ref<Texture2D> m_GBufferEmission;
        Ref<Texture2D> m_GBufferInstanceUUID;
        Ref<Texture2D> m_GBufferDepthStencil;

        Ref<Texture2D> m_HDR_ResolveTexture;
        Ref<Texture2D> m_LDR_ResolveTexture;
        Ref<Texture2D> m_GBufferGeometrySurfaceNormalsCopy;
        Ref<Texture2D> m_GBufferInstanceUUIDCopy;
        Ref<Texture2D> m_GBufferDepthStencilCopy;

        Ref<Framebuffer> m_GBuffer_PBR_StaticFramebuffer;
        Ref<Pipeline> m_GBuffer_PBR_StaticPipeline;
        Ref<Pipeline> m_GBuffer_PBR_Static_DissolutionPipeline;
        
        Ref<Pipeline> m_GBuffer_PBR_Static_IncinerationPipeline;
        Ref<PipelineCompute> m_IncinerationParticlesUpdateIndirectArgsPipelineCompute;
        Ref<PipelineCompute> m_IncinerationParticlesUpdatePipelineCompute;
        Ref<PipelineCompute> m_IncinerationParticlesUpdateAuxiliaryPipelineCompute;
        Ref<Pipeline> m_IncinerationParticlesInfluencePipeline;
        Ref<Pipeline> m_IncinerationParticlesPipeline;
        Ref<StructuredBuffer> m_SBIncinerationParticles;
        Ref<PrimitiveBuffer> m_PBIncinerationParticleRangeBuffer;
        Ref<Texture2D> m_IncinerationParticlesSparkTexture;

        Ref<Framebuffer> m_GBuffer_EmissionFramebuffer;
        Ref<Pipeline> m_GBuffer_EmissionPipeline;

        Ref<Framebuffer> m_GBuffer_DecalFramebuffer;
        Ref<Pipeline> m_GBuffer_DecalPipeline;
        Ref<VertexBuffer> m_DecalsTransformBuffer;
        Ref<VertexBuffer> m_DecalsInstanceBuffer;
        Ref<Texture2D> m_DecalNormalAlpha;

        Ref<Framebuffer> m_HDR_ResolvePBR_StaticFramebuffer;
        Ref<Pipeline> m_GBufferResolve_PBR_StaticPipeline;
        
        Ref<Framebuffer> m_HDR_ResolveEmissionFramebuffer;
        Ref<Pipeline> m_GBufferResolve_EmissionPipeline;

        Ref<Framebuffer> m_HDR_ResolveFramebuffer;
        Ref<Pipeline> m_SkyboxPipeline;

        Ref<Framebuffer> m_LDR_ResolveFramebuffer;
        Ref<Pipeline> m_HDR_To_LDRPipeline;

        Ref<Framebuffer> m_FXAAFramebuffer;
        Ref<Pipeline> m_FXAAPipeline;

        Ref<Framebuffer> m_DirectionalShadowMapFramebuffer;
        Ref<Pipeline> m_DirectionalShadowMapPipeline;
        Ref<Pipeline> m_DirectionalShadowMapDissolutionPipeline;
        Ref<Pipeline> m_DirectionalShadowMapIncinirationPipeline;

        Ref<Framebuffer> m_PointShadowMapFramebuffer;
        Ref<Pipeline> m_PointShadowMapPipeline;
        Ref<Pipeline> m_PointShadowMapDissolutionPipeline;
        Ref<Pipeline> m_PointShadowMapIncinirationPipeline;

        Ref<Framebuffer> m_SpotShadowMapFramebuffer;
        Ref<Pipeline> m_SpotShadowMapPipeline;
        Ref<Pipeline> m_SpotShadowMapDissolutionPipeline;
        Ref<Pipeline> m_SpotShadowMapIncinirationPipeline;

        Ref<VertexBuffer> m_SmokeParticlesInstanceBuffer;
        Ref<Pipeline> m_SmokeParticlePipeline;
        Ref<Texture2D> m_SmokeParticlesRLU;
        Ref<Texture2D> m_SmokeParticlesDBF;
        Ref<Texture2D> m_SmokeParticlesEMVA;

        uint32_t m_ViewportWidth;
        uint32_t m_ViewportHeight;
    };
}