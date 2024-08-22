#pragma once
#include "DLEngine/Renderer/Pipeline.h"
#include "DLEngine/Renderer/Scene.h"
#include "DLEngine/Renderer/StructuredBuffer.h"

namespace DLEngine
{
    struct CBCamera
    {
        Math::Mat4x4 Projection;
        Math::Mat4x4 InvProjection;
        Math::Mat4x4 View;
        Math::Mat4x4 InvView;
        Math::Mat4x4 ViewProjection;
        Math::Mat4x4 InvViewProjection;
        Math::Vec3 CameraPosition;
        float _padding1{ 0.0f };
        Math::Vec3 BL;
        float _padding2{ 0.0f };
        Math::Vec3 BL2TL;
        float _padding3{ 0.0f };
        Math::Vec3 BL2BR;
        float _padding4{ 0.0f };
    };

    struct CBPBRSettings
    {
        Math::Vec3 IndirectLightRadiance{ 0.1f };
        float OverwrittenRoughness{ 0.5f };
        uint32_t OverwriteRoughness{ static_cast<uint32_t>(false) };
        uint32_t UseIBL{ static_cast<uint32_t>(true) };
        uint32_t UseDiffuseReflections{ static_cast<uint32_t>(true) };
        uint32_t UseSpecularReflections{ static_cast<uint32_t>(true) };
    };

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

    struct CBLightsCount
    {
        uint32_t DirectionalLightsCount{ 0u };
        uint32_t PointLightsCount{ 0u };
        uint32_t SpotLightsCount{ 0u };
        float _padding{ 0.0f };
    };

    struct CBPostProcessSettings
    {
        float EV100{ 0.0f };
        float Gamma{ 2.2f };
        float _padding[2u];
    };

    struct SceneEnvironment
    {
        Ref<TextureCube> Skybox;
        Ref<TextureCube> IrradianceMap;
        Ref<TextureCube> PrefilteredMap;
    };

    struct ShadowMapSettings
    {
        uint32_t MapSize{ 2048u };
        bool UseDirectionalShadows{ true };
        float DirectionalLightShadowMargin{ 5.0f };
        float DirectionalLightShadowDistance{ 10.0f };
        bool UseOmnidirectionalShadows{ true };
        bool UseSpotShadows{ true };
        bool UsePCF{ true };
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
        float WorldTexelSize{ Math::Numeric::Inf };
    };

    struct SpotLightShadowMapData
    {
        Camera POV;
        float WorldTexelSize{ Math::Numeric::Inf };
    };

    struct SceneShadowEnvironment
    {
        std::vector<DirectionalLightShadowMapData> DirectionalLightsData;
        std::vector<PointLightShadowMapData> PointLightsData;
        std::vector<SpotLightShadowMapData> SpotLightsData;
      
        Ref<StructuredBuffer> SBDirectionalLightsPOVs;
        Ref<Texture2D> DirectionalShadowMaps;
        Ref<ConstantBuffer> CBDirectionalLightData;

        Ref<StructuredBuffer> SBPointLightsPOVs;
        Ref<TextureCube> PointShadowMaps;
        Ref<ConstantBuffer> CBPointLightData;

        Ref<StructuredBuffer> SBSpotLightsPOVs;
        Ref<Texture2D> SpotShadowMaps;
        Ref<ConstantBuffer> CBSpotLightData;

        ShadowMapSettings Settings;

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

        void SetViewportSize(uint32_t width, uint32_t height) { m_ViewportWidth = width; m_ViewportHeight = height; }

        void RenderScene(const Ref<Scene>& scene);
        void SetPBRSettings(const CBPBRSettings& pbrSettings) { m_CBPBRSettings->SetData(Buffer{ &pbrSettings, sizeof(CBPBRSettings) }); }
        void SetPostProcessSettings(const CBPostProcessSettings& postProcessSettings) { m_CBPostProcessSettings->SetData(Buffer{ &postProcessSettings, sizeof(CBPostProcessSettings) }); }
        void SetShadowMapSettings(const ShadowMapSettings& shadowMapSettings);

    private:
        void Init();

        void PreRender();

        void ShadowPass();
        void GeometryPass();
        void SkyboxPass();
        void PostProcessPass();

        void UpdateCBCamera(const Camera& camera);
        void UpdateDirectionalLightsData();
        void UpdatePointLightsData();
        void UpdateSpotLightsData();

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

        Ref<Framebuffer> m_MainFramebuffer;
        
        Ref<Pipeline> m_PBRStaticPipeline;

        Ref<Pipeline> m_EmissionPipeline;
        
        Ref<Pipeline> m_PostProcessPipeline;
        Ref<Material> m_PostProcessMaterial;

        Ref<Pipeline> m_SkyboxPipeline;
        Ref<Material> m_SkyboxMaterial;

        Ref<StructuredBuffer> m_SBDirectionalLights;
        Ref<StructuredBuffer> m_SBPointLights;
        Ref<StructuredBuffer> m_SBSpotLights;

        Ref<Pipeline> m_DirectionalShadowMapPipeline;
        Ref<Framebuffer> m_DirectionalShadowMapFramebuffer;

        Ref<Pipeline> m_PointShadowMapPipeline;
        Ref<Framebuffer> m_PointShadowMapFramebuffer;

        Ref<Pipeline> m_SpotShadowMapPipeline;
        Ref<Framebuffer> m_SpotShadowMapFramebuffer;

        uint32_t m_ViewportWidth;
        uint32_t m_ViewportHeight;
    };
}