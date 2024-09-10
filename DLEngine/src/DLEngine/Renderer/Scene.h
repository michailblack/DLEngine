#pragma once
#include "DLEngine/Core/Events/MouseEvent.h"

#include "DLEngine/Renderer/Mesh/MeshRegistry.h"

#include "DLEngine/Renderer/CameraController.h"
#include "DLEngine/Renderer/IDragger.h"

#include "DLEngine/Utils/DeltaTime.h"
#include "DLEngine/Utils/RandomGenerator.h"

namespace DLEngine
{
    struct DirectionalLight
    {
        Math::Vec3 Direction;
        Math::Vec3 Radiance;
        float SolidAngle;
    };

    struct PointLight
    {
        Math::Vec3 Position;
        Math::Vec3 Radiance;
        float Radius;
    };

    struct SpotLight
    {
        Math::Vec3 Position;
        Math::Vec3 Direction;
        Math::Vec3 Radiance;
        float Radius;
        float InnerCutoffCos;
        float OuterCutoffCos;
    };

    struct LightEnvironment
    {
        std::vector<DirectionalLight> DirectionalLights;
        std::vector<std::pair<PointLight, MeshRegistry::MeshUUID>> PointLights;
        std::vector<std::pair<SpotLight, MeshRegistry::MeshUUID>> SpotLights;
    };

    struct SmokeParticle
    {
        Math::Vec3 Position{ 0.0f };
        Math::Vec3 VelocityPerSecond{ 0.0f };
        float Rotation{ RandomGenerator::GenerateRandom(-Math::Numeric::Pi, Math::Numeric::Pi) };
        float LifetimeMS{ Math::Numeric::Max };
        float LifetimePassedMS{ 0.0f };
    };

    struct SmokeEmitter
    {
        std::vector<SmokeParticle> Particles;
        Math::Vec3 Position;
        Math::Vec3 SpawnedParticleTintColor;
        Math::Vec2 InitialParticleSize;
        Math::Vec2 FinalParticleSize;
        float ParticleEmissionIntensity;
        float ParticleSpawnRadius;
        float MinParticleLifetimeMS;
        float MaxParticleLifetimeMS;
        float ParticleVerticalVelocity;
        float ParticleHorizontalVelocity;
        uint32_t ParticleSpawnRatePerSecond;
    };

    struct SmokeEnvironment
    {
        using EmitterIndex  = uint32_t;
        using ParticleIndex = uint32_t;

        std::vector<std::pair<SmokeEmitter, MeshRegistry::MeshUUID>> SmokeEmitters;
        std::vector<std::pair<EmitterIndex, ParticleIndex>> SortedSmokeParticles;
    };

    struct SceneSpecification
    {
        CameraController::CameraResizeCallbackFn CameraResizeCallback;
        std::string SceneName{ "Untitled Scene" };
        uint32_t ViewportWidth{ 0u };
        uint32_t ViewportHeight{ 0u };
    };

    struct Decal
    {
        Math::Mat4x4 DecalToMesh;
        Ref<Instance> DecalInstance;
        MeshRegistry::MeshUUID ParentMeshUUID;
    };

    class Scene
    {
    public:
        Scene(const SceneSpecification& specification);

        void OnUpdate(DeltaTime dt);
        void OnEvent(Event& e);

        void AddDirectionalLight(const Math::Vec3& direction, const Math::Vec3& radiance, float solidAngle);

        void AddPointLight(const Math::Vec3& position, const Math::Vec3& irradiance, float radius, float distance, const Math::Vec3& emissionMeshTranslation);
        void AddPointLight(const Math::Vec3& position, const Math::Vec3& irradiance, float radius, float distance, const MeshRegistry::MeshUUID& meshInstance);

        void AddSpotLight(const Math::Vec3& position, const Math::Vec3& direction, float radius, float innerCutoffCos, float outerCutoffCos, const Math::Vec3& irradiance, float distance, const Math::Vec3& emissionMeshTranslation);
        void AddSpotLight(const Math::Vec3& position, const Math::Vec3& direction, float radius, float innerCutoffCos, float outerCutoffCos, const Math::Vec3& irradiance, float distance, const MeshRegistry::MeshUUID& meshInstance);

        void AddSmokeEmitter(const SmokeEmitter& emitter, const Math::Vec3& emissionMeshTranslation);
        void AddSmokeEmitter(const SmokeEmitter& emitter, const MeshRegistry::MeshUUID& meshInstance);

        void ClearSmokeEmitters();

        void SpawnDecal(const Math::Ray& ray, const Math::Vec3& tintColor, float rotation);

        uint32_t GetViewportWidth() const noexcept { return m_ViewportWidth; }
        uint32_t GetViewportHeight() const noexcept { return m_ViewportHeight; }

        const Camera& GetCamera() const noexcept { return m_SceneCameraController.GetCamera(); }

        MeshRegistry& GetMeshRegistry() noexcept { return m_MeshRegistry; }
        const MeshRegistry& GetMeshRegistry() const noexcept { return m_MeshRegistry; }

        uint32_t GetOverallParticlesCount() const noexcept { return static_cast<uint32_t>(m_SmokeEnvironment.SortedSmokeParticles.size()); }

    private:
        void UpdateSmokeEmitters(DeltaTime dt);
        void SortSmokeParticles();

    private:
        bool OnWindowResize(WindowResizeEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);

    private:
        CameraController m_SceneCameraController;
        MeshRegistry m_MeshRegistry;

        LightEnvironment m_LightEnvironment;
        SmokeEnvironment m_SmokeEnvironment;

        std::vector<Decal> m_Decals;

        std::string m_SceneName;

        Scope<IDragger> m_Dragger;

        uint32_t m_ViewportWidth;
        uint32_t m_ViewportHeight;

    private:
        friend class SceneRenderer;
    };

    namespace Utils
    {
        Math::Vec3 SphereLightRadianceFromIrradiance(const Math::Vec3& irradiance, float radius, float distance);
        
        // Threshold is basically (irradiance / radiance)
        float SphereLightContributionDistance(float threshold, float radius);
    }
}