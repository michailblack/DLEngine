#include "dlpch.h"
#include "Scene.h"

#include "DLEngine/Renderer/Renderer.h"
#include "DLEngine/Renderer/SceneRenderer.h"

#include "DLEngine/Utils/RadixSort.h"

#include <execution>

namespace DLEngine
{

    Scene::Scene(const SceneSpecification& specification)
        : m_SceneName(specification.SceneName)
        , m_ViewportWidth(specification.ViewportWidth)
        , m_ViewportHeight(specification.ViewportHeight)
    {
        m_SceneCameraController.SetCameraResizeCallback(specification.CameraResizeCallback);
        
        WindowResizeEvent e{ m_ViewportWidth, m_ViewportHeight };
        m_SceneCameraController.OnEvent(e);
    }

    void Scene::OnUpdate(DeltaTime dt)
    {
        m_SceneCameraController.OnUpdate(dt);

        if (m_Dragger)
        {
            const auto& camera{ m_SceneCameraController.GetCamera() };
            const auto& cursorPos{ Input::GetCursorPosition() };
            Math::Vec3 cursorPosNDC{
                cursorPos.x / static_cast<float>(m_ViewportWidth) * 2.0f - 1.0f,
                (1.0f - cursorPos.y / static_cast<float>(m_ViewportHeight)) * 2.0f - 1.0f,
                1.0f
            };

            Math::Ray ray{};
            ray.Origin = camera.ConstructFrustumPos(cursorPosNDC);
            ray.Direction = Math::Normalize(camera.ConstructFrustumPosNoTranslation(cursorPosNDC));

            m_Dragger->Drag(ray);
        }

        UpdateSmokeEmitters(dt);
        SortSmokeParticles();
    }

    void Scene::OnEvent(Event& e)
    {
        m_SceneCameraController.OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>(DL_BIND_EVENT_FN(Scene::OnWindowResize));
        dispatcher.Dispatch<MouseButtonPressedEvent>(DL_BIND_EVENT_FN(Scene::OnMouseButtonPressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(DL_BIND_EVENT_FN(Scene::OnMouseButtonReleased));
    }

    void Scene::AddDirectionalLight(const Math::Vec3& direction, const Math::Vec3& radiance, float solidAngle)
    {
        DirectionalLight directionalLight{};
        directionalLight.Direction = Math::Normalize(direction);
        directionalLight.Radiance = radiance;
        directionalLight.SolidAngle = solidAngle;

        m_LightEnvironment.DirectionalLights.emplace_back(directionalLight);
    }

    void Scene::AddPointLight(const Math::Vec3& position, const Math::Vec3& irradiance, float radius, float distance, const Math::Vec3& emissionMeshTranslation)
    {
        const auto& emissionShader{ Renderer::GetShaderLibrary()->Get("Emission") };
        const auto& unitSphere{ Renderer::GetMeshLibrary()->Get("UNIT_SPHERE") };

        const auto& material{ Material::Create(emissionShader) };
        const auto& instance{ Instance::Create(emissionShader) };

        const auto transform{ Math::Mat4x4::Scale(Math::Vec3{ radius }) * Math::Mat4x4::Translate(emissionMeshTranslation) };
        Math::Vec3 radiance{ Utils::SphereLightRadianceFromIrradiance(irradiance, radius, distance) };
        instance->Set("TRANSFORM", Buffer{ &transform, sizeof(Math::Mat4x4) });
        instance->Set("RADIANCE", Buffer{ &radiance, sizeof(Math::Vec3) });

        m_MeshRegistry.AddSubmesh(unitSphere, 0u, material, instance);
        AddPointLight(position, irradiance, radius, distance, instance);
    }
    void Scene::AddPointLight(const Math::Vec3& position, const Math::Vec3& irradiance, float radius, float distance, const Ref<Instance>& meshInstance)
    {
        DL_ASSERT(meshInstance->HasUniform("TRANSFORM"), "Mesh instance must have a transform");
        DL_ASSERT(radius > 0.0f, "Radius must be greater than 0.0f");
        DL_ASSERT(distance > 0.0f, "Distance must be greater than 0.0f");

        PointLight pointLight{};
        pointLight.Position = position;
        pointLight.Radiance = Utils::SphereLightRadianceFromIrradiance(irradiance, radius, distance);
        pointLight.Radius = radius;

        m_LightEnvironment.PointLights.emplace_back(pointLight, meshInstance);
    }

    void Scene::AddSpotLight(const Math::Vec3& position, const Math::Vec3& direction, float radius, float innerCutoffCos, float outerCutoffCos, const Math::Vec3& irradiance, float distance, const Math::Vec3& emissionMeshTranslation)
    {
        const auto& emissionShader{ Renderer::GetShaderLibrary()->Get("Emission") };
        const auto& unitSphere{ Renderer::GetMeshLibrary()->Get("UNIT_SPHERE") };

        const auto& material{ Material::Create(emissionShader) };
        const auto& instance{ Instance::Create(emissionShader) };

        const auto transform{ Math::Mat4x4::Scale(Math::Vec3{ radius }) * Math::Mat4x4::Translate(emissionMeshTranslation) };
        Math::Vec3 radiance{ Utils::SphereLightRadianceFromIrradiance(irradiance, radius, distance) };
        instance->Set("TRANSFORM", Buffer{ &transform, sizeof(Math::Mat4x4) });
        instance->Set("RADIANCE", Buffer{ &radiance, sizeof(Math::Vec3) });

        m_MeshRegistry.AddSubmesh(unitSphere, 0u, material, instance);
        AddSpotLight(position, direction, radius, innerCutoffCos, outerCutoffCos, irradiance, distance, instance);
    }

    void Scene::AddSpotLight(const Math::Vec3& position, const Math::Vec3& direction, float radius, float innerCutoffCos, float outerCutoffCos, const Math::Vec3& irradiance, float distance, const Ref<Instance>& meshInstance)
    {
        DL_ASSERT(meshInstance->HasUniform("TRANSFORM"), "Mesh instance must have a transform");
        DL_ASSERT(radius > 0.0f, "Radius must be greater than 0.0f");
        DL_ASSERT(distance > 0.0f, "Distance must be greater than 0.0f");
        DL_ASSERT(innerCutoffCos >= -1.0f && innerCutoffCos <= 1.0f, "Inner cutoff cosine must be in the range [-1.0f, 1.0f]");
        DL_ASSERT(outerCutoffCos >= -1.0f && outerCutoffCos <= 1.0f, "Outer cutoff cosine must be in the range [-1.0f, 1.0f]");

        SpotLight spotLight{};
        spotLight.Position = position;
        spotLight.Direction = Math::Normalize(direction);
        spotLight.Radiance = Utils::SphereLightRadianceFromIrradiance(irradiance, radius, distance);
        spotLight.Radius = radius;
        spotLight.InnerCutoffCos = innerCutoffCos;
        spotLight.OuterCutoffCos = outerCutoffCos;

        m_LightEnvironment.SpotLights.emplace_back(spotLight, meshInstance);
    }

    void Scene::AddSmokeEmitter(const SmokeEmitter& emitter, const Math::Vec3& emissionMeshTranslation)
    {
        const auto& emissionShader{ Renderer::GetShaderLibrary()->Get("Emission") };
        const auto& unitSphere{ Renderer::GetMeshLibrary()->Get("UNIT_SPHERE") };

        const auto& material{ Material::Create(emissionShader) };
        const auto& instance{ Instance::Create(emissionShader) };

        const auto transform{ Math::Mat4x4::Scale(Math::Vec3{ 0.01f }) * Math::Mat4x4::Translate(emissionMeshTranslation) };
        instance->Set("TRANSFORM", Buffer{ &transform, sizeof(Math::Mat4x4) });
        instance->Set("RADIANCE", Buffer{ &emitter.SpawnedParticleTintColor, sizeof(Math::Vec3) });

        m_MeshRegistry.AddSubmesh(unitSphere, 0u, material, instance);
        AddSmokeEmitter(emitter, instance);
    }

    void Scene::AddSmokeEmitter(const SmokeEmitter& emitter, const Ref<Instance>& meshInstance)
    {
        m_SmokeEnvironment.SmokeEmitters.emplace_back(emitter, meshInstance);
    }

    void Scene::ClearSmokeEmitters()
    {
        const auto& unitSphere{ Renderer::GetMeshLibrary()->Get("UNIT_SPHERE") };
        const auto& emissionShader{ Renderer::GetShaderLibrary()->Get("Emission") };
        const auto& material{ Material::Create(emissionShader) };

        for (const auto& smokeEmitterData : m_SmokeEnvironment.SmokeEmitters)
            m_MeshRegistry.RemoveSubmesh(unitSphere, 0u, material, smokeEmitterData.second);

        m_SmokeEnvironment.SmokeEmitters.clear();
    }

    void Scene::UpdateSmokeEmitters(DeltaTime dt)
    {
        std::for_each(std::execution::par_unseq, m_SmokeEnvironment.SmokeEmitters.begin(), m_SmokeEnvironment.SmokeEmitters.end(),
            [dt, &sceneCamera{ std::as_const(m_SceneCameraController.GetCamera()) }]
            (auto& smokeEmitterData)
            {
                SmokeEmitter& smokeEmitter{ smokeEmitterData.first };
                const auto& instance{ smokeEmitterData.second };

                smokeEmitter.Particles.erase(std::remove_if(std::execution::par_unseq, smokeEmitter.Particles.begin(), smokeEmitter.Particles.end(),
                    [dt](SmokeParticle& particle)
                    {
                        particle.LifetimePassedMS += dt;
                        particle.Position += particle.VelocityPerSecond * dt.GetSeconds();
                        return particle.LifetimePassedMS > particle.LifetimeMS;
                    }),
                    smokeEmitter.Particles.end()
                );

                const uint32_t particlesToSpawn{ static_cast<uint32_t>(dt.GetSeconds() * static_cast<float>(smokeEmitter.ParticleSpawnRatePerSecond)) };

                const auto& transform{ instance->Get<Math::Mat4x4>("TRANSFORM") };
                const auto& smokeEmitterWorldPos{ Math::PointToSpace(smokeEmitter.Position, transform) };

                const uint32_t beginSpawnIndex{ static_cast<uint32_t>(smokeEmitter.Particles.size()) };
                smokeEmitter.Particles.resize(smokeEmitter.Particles.size() + particlesToSpawn);
                std::generate(std::execution::par_unseq, smokeEmitter.Particles.begin() + beginSpawnIndex, smokeEmitter.Particles.end(),
                    [&smokeEmitter{ std::as_const(smokeEmitter) }, &smokeEmitterWorldPos{ std::as_const(smokeEmitterWorldPos) }, &sceneCamera{ std::as_const(sceneCamera) }]()
                    {
                        SmokeParticle particle{};

                        const float theta{ RandomGenerator::GenerateRandomInRange(0.0f, 2.0f * Math::Numeric::Pi) };
                        const float phi{ RandomGenerator::GenerateRandomInRange(0.0f, Math::Numeric::Pi) };
                        const float distance{ RandomGenerator::GenerateRandomInRange(0.0f, smokeEmitter.ParticleSpawnRadius) };

                        const Math::Vec3 emitterRelativePos{
                            distance * Math::SinEst(phi) * Math::CosEst(theta),
                            distance * Math::SinEst(phi) * Math::SinEst(theta),
                            distance * Math::CosEst(phi)
                        };

                        particle.Position = smokeEmitterWorldPos + emitterRelativePos;

                        const auto& cameraUp{ sceneCamera.GetUp() };
                        const auto& cameraRight{ sceneCamera.GetRight() };
                        const auto& cameraForward{ sceneCamera.GetForward() };

                        const float particleRightVelocity{
                            RandomGenerator::GenerateRandomInRange(-smokeEmitter.ParticleHorizontalVelocity, smokeEmitter.ParticleHorizontalVelocity)
                        };
                        const float particleForwardVelocity{
                            RandomGenerator::GenerateRandomInRange(-smokeEmitter.ParticleHorizontalVelocity, smokeEmitter.ParticleHorizontalVelocity)
                        };

                        particle.VelocityPerSecond = cameraUp * smokeEmitter.ParticleVerticalVelocity +
                            cameraRight * particleRightVelocity +
                            cameraForward * particleForwardVelocity;

                        particle.LifetimeMS = RandomGenerator::GenerateRandomInRange(
                            smokeEmitter.MinParticleLifetimeMS,
                            smokeEmitter.MaxParticleLifetimeMS
                        );

                        return particle;
                    }
                );
            }
        );
    }

    void Scene::SortSmokeParticles()
    {
        using SmokeParticleID = std::pair<SmokeEnvironment::EmitterIndex, SmokeEnvironment::ParticleIndex>;

        const auto& camera{ m_SceneCameraController.GetCamera() };
        const auto& cameraPos{ camera.GetPosition() };
        const Math::Vec3& particlePlaneNormal{ -camera.GetForward() };
        
        std::unordered_map<float, SmokeParticleID> distancesToParticles{};
        std::vector<float> particleDistances{};
        for (uint32_t emitterIndex{ 0u }; emitterIndex < m_SmokeEnvironment.SmokeEmitters.size(); ++emitterIndex)
        {
            const auto& smokeEmitter{ m_SmokeEnvironment.SmokeEmitters[emitterIndex].first };

            distancesToParticles.reserve(distancesToParticles.size() + smokeEmitter.Particles.size());
            particleDistances.reserve(particleDistances.size() + smokeEmitter.Particles.size());

            for (uint32_t particleIndex{ 0u }; particleIndex < smokeEmitter.Particles.size(); ++particleIndex)
            {
                const auto& particle{ smokeEmitter.Particles[particleIndex] };

                // Plane equation: Ax + By + Cz + D = 0, where (A, B, C) is its normal.
                const float D{ -Math::Dot(particle.Position, particlePlaneNormal) };
                const float distanceToPlane{ Math::Dot(cameraPos, particlePlaneNormal) + D };

                if (distanceToPlane < 0.0f)
                    continue;

                particleDistances.emplace_back(distanceToPlane);
                distancesToParticles[distanceToPlane] = std::make_pair(emitterIndex, particleIndex);
            }
        }

        const uint32_t particlesCount{ static_cast<uint32_t>(particleDistances.size()) };
        
        std::vector<float> sortedParticleDistances(particlesCount);
        Utils::RadixSort11(particleDistances.data(), sortedParticleDistances.data(), particlesCount);

        m_SmokeEnvironment.SortedSmokeParticles.reserve(particlesCount);
        m_SmokeEnvironment.SortedSmokeParticles.clear();

        for (const auto& paricleDistance : sortedParticleDistances | std::views::reverse)
            m_SmokeEnvironment.SortedSmokeParticles.emplace_back(distancesToParticles[paricleDistance]);
    }

    bool Scene::OnWindowResize(WindowResizeEvent& e)
    {
        m_ViewportWidth = e.GetWidth();
        m_ViewportHeight = e.GetHeight();

        return false;
    }

    bool Scene::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        switch (e.GetButton())
        {
        case VK_RBUTTON:
        {
            const auto& camera{ m_SceneCameraController.GetCamera() };
            const auto& cursorPos{ Input::GetCursorPosition() };
            Math::Vec3 cursorPosNDC{
                cursorPos.x / static_cast<float>(m_ViewportWidth) * 2.0f - 1.0f,
                (1.0f - cursorPos.y / static_cast<float>(m_ViewportHeight)) * 2.0f - 1.0f,
                1.0f
            };

            Math::Ray ray{};
            ray.Origin = camera.ConstructFrustumPos(cursorPosNDC);
            ray.Direction = Math::Normalize(camera.ConstructFrustumPosNoTranslation(cursorPosNDC));

            MeshRegistry::IntersectInfo intersectInfo{};
            if (Math::Intersects(ray, m_MeshRegistry, intersectInfo))
            {
                m_Dragger.reset(new InstanceDragger{
                    intersectInfo.SubmeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint,
                    intersectInfo.SubmeshIntersectInfo.TriangleIntersectInfo.T,
                    intersectInfo.SubmeshID.Instance
                });
            }
        } break;
        default:
            break;
        }

        return false;
    }

    bool Scene::OnMouseButtonReleased(MouseButtonReleasedEvent& e)
    {
        switch (e.GetButton())
        {
        case VK_RBUTTON:
            m_Dragger.reset();
            break;
        default:
            break;
        }

        return false;
    }

    namespace Utils
    {
        Math::Vec3 SphereLightRadianceFromIrradiance(const Math::Vec3& irradiance, float radius, float distance)
        {
            const float sizeRelation{ radius / distance };
            const float denom{ 1.0f - Math::Sqrt(1.0f - sizeRelation * sizeRelation) };
            
            return irradiance / denom;
        }

        float SphereLightContributionDistance(float threshold, float radius)
        {
            const float a{ 1.0f - threshold };
            const float denom{ Math::Sqrt(1.0f - a * a) };

            return radius / denom;
        }

    }

}