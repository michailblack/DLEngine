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
        std::vector<std::pair<PointLight, Ref<Instance>>> PointLights;
        std::vector<std::pair<SpotLight, Ref<Instance>>> SpotLights;
    };

    struct SmokeParticle
    {
        Math::Vec3 Position{ 0.0f };
        Math::Vec3 VelocityPerSecond{ 0.0f };
        float Rotation{ RandomGenerator::GenerateRandomInRange(-Math::Numeric::Pi, Math::Numeric::Pi) };
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
        uint32_t ParticleSpawnRatePerSecond;
    };

    struct SmokeEnvironment
    {
        using EmitterIndex  = uint32_t;
        using ParticleIndex = uint32_t;

        std::vector<std::pair<SmokeEmitter, Ref<Instance>>> SmokeEmitters;
        std::vector<std::pair<EmitterIndex, ParticleIndex>> SortedSmokeParticles;
    };

    struct SceneSpecification
    {
        CameraController::CameraResizeCallbackFn CameraResizeCallback;
        std::string SceneName{ "Untitled Scene" };
        uint32_t ViewportWidth{ 0u };
        uint32_t ViewportHeight{ 0u };
    };

    class Scene
    {
    public:
        Scene(const SceneSpecification& specification);

        void OnUpdate(DeltaTime dt);
        void OnEvent(Event& e);

        void AddDirectionalLight(const Math::Vec3& direction, const Math::Vec3& radiance, float solidAngle);

        void AddPointLight(const Math::Vec3& position, const Math::Vec3& irradiance, float radius, float distance, const Math::Vec3& emissionMeshTranslation);
        void AddPointLight(const Math::Vec3& position, const Math::Vec3& irradiance, float radius, float distance, const Ref<Instance>& meshInstance);

        void AddSpotLight(const Math::Vec3& position, const Math::Vec3& direction, float radius, float innerCutoffCos, float outerCutoffCos, const Math::Vec3& irradiance, float distance, const Math::Vec3& emissionMeshTranslation);
        void AddSpotLight(const Math::Vec3& position, const Math::Vec3& direction, float radius, float innerCutoffCos, float outerCutoffCos, const Math::Vec3& irradiance, float distance, const Ref<Instance>& meshInstance);

        void AddSmokeEmitter(const SmokeEmitter& emitter, const Math::Vec3& emissionMeshTranslation);
        void AddSmokeEmitter(const SmokeEmitter& emitter, const Ref<Instance>& meshInstance);

        void ClearSmokeEmitters();

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

        LightEnvironment m_LightEnvironment;

        MeshRegistry m_MeshRegistry;

        std::string m_SceneName;

        SmokeEnvironment m_SmokeEnvironment;

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