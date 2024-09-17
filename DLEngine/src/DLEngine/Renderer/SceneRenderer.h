#pragma once
#include "DLEngine/Renderer/Pipeline.h"
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

        void PreRender();

        void ShadowPass();
        void GeometryPass();
        void SkyboxPass();
        void SmokeParticlesPass();
        void PostProcessPass();

        void UpdateCBCamera(const Camera& camera);
        void UpdateDirectionalLightsData();
        void UpdatePointLightsData();
        void UpdateSpotLightsData();

        void UpdateSmokeParticlesData();

        void BuildIrradianceMap();
        void BuildPrefilteredMap();

    private:
        SceneShadowEnvironment m_SceneShadowEnvironment;

        SceneEnvironment m_SceneEnvironment;
        
        Ref<Scene> m_Scene;

        Ref<ConstantBuffer> m_CBCamera;
        Ref<ConstantBuffer> m_CBPBRSettings;
        Ref<ConstantBuffer> m_CBShadowMappingData;
        Ref<ConstantBuffer> m_CBLightsCount;
        Ref<ConstantBuffer> m_CBPostProcessSettings;
        Ref<ConstantBuffer> m_CBTextureAtlasData;

        Ref<Framebuffer> m_MainFramebuffer;
        
        Ref<Pipeline> m_PBRStaticPipeline;
        Ref<Pipeline> m_DissolutionPipeline;
        Ref<Pipeline> m_EmissionPipeline;
        Ref<Pipeline> m_PostProcessPipeline;
        Ref<Pipeline> m_SkyboxPipeline;

        Ref<StructuredBuffer> m_SBDirectionalLights;
        Ref<StructuredBuffer> m_SBPointLights;
        Ref<StructuredBuffer> m_SBSpotLights;

        Ref<Pipeline> m_DirectionalShadowMapPipeline;
        Ref<Framebuffer> m_DirectionalShadowMapFramebuffer;

        Ref<Pipeline> m_PointShadowMapPipeline;
        Ref<Framebuffer> m_PointShadowMapFramebuffer;

        Ref<Pipeline> m_SpotShadowMapPipeline;
        Ref<Framebuffer> m_SpotShadowMapFramebuffer;

        Ref<VertexBuffer> m_SmokeParticlesInstanceBuffer;
        Ref<Pipeline> m_SmokeParticlePipeline;
        Ref<Texture2D> m_SmokeParticlesRLU;
        Ref<Texture2D> m_SmokeParticlesDBF;
        Ref<Texture2D> m_SmokeParticlesEMVA;

        Ref<Pipeline> m_MaxDepthPipeline;
        Ref<Framebuffer> m_MaxDepthFramebuffer;

        // Used for unbinding textures
        Ref<Texture2D> m_NullTexture;

        uint32_t m_ViewportWidth;
        uint32_t m_ViewportHeight;

        const uint32_t m_SamplesCount{ 4u };
    };
}