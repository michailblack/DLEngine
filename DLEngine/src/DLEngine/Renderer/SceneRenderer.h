#pragma once
#include "DLEngine/Renderer/Pipeline.h"
#include "DLEngine/Renderer/Scene.h"
#include "DLEngine/Renderer/StructuredBuffer.h"

namespace DLEngine
{
    struct PBRSettings
    {
        Math::Vec3 IndirectLightRadiance{ 0.1f };
        float OverwrittenRoughness{ 0.5f };
        uint32_t OverwriteRoughness{ static_cast<uint32_t>(false) };
        uint32_t UseIBL{ static_cast<uint32_t>(true) };
        uint32_t UseDiffuseReflections{ static_cast<uint32_t>(true) };
        uint32_t UseSpecularReflections{ static_cast<uint32_t>(true) };
    };

    struct PostProcessSettings
    {
        float EV100{ 0.0f };
        float Gamma{ 2.2f };
        float _Padding[2u];
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
        float _padding1{ 0.0f };
        Math::Vec3 BL;
        float _padding2{ 0.0f };
        Math::Vec3 BL2TL;
        float _padding3{ 0.0f };
        Math::Vec3 BL2BR;
        float _padding4{ 0.0f };
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

    struct SceneEnvironment
    {
        Ref<TextureCube> Skybox;
        Ref<TextureCube> IrradianceMap;
        Ref<TextureCube> PrefilteredMap;
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

        void RenderScene(const Ref<Scene>& scene, const PBRSettings& pbrSettings, const PostProcessSettings& postProcessSettings);

    private:
        void Init();

        void PreRender();

        void GeometryPass();
        void SkyboxPass();
        void PostProcessPass();

        void UpdateCBCamera(const Camera& camera);
        void SubmitAllMeshesForShader(const std::string_view shaderName);

        void BuildIrradianceMap();
        void BuildPrefilteredMap();

    private:
        SceneEnvironment m_SceneEnvironment;
        
        Ref<Scene> m_Scene;

        Ref<ConstantBuffer> m_CBCamera;
        Ref<ConstantBuffer> m_CBPBRSettings;
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

        uint32_t m_ViewportWidth;
        uint32_t m_ViewportHeight;
    };
}