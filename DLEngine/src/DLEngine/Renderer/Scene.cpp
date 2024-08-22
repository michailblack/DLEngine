#include "dlpch.h"
#include "Scene.h"

#include "DLEngine/Renderer/Renderer.h"

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
    }

    void Scene::OnEvent(Event& e)
    {
        m_SceneCameraController.OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>(DL_BIND_EVENT_FN(Scene::OnWindowResize));
        dispatcher.Dispatch<MouseButtonPressedEvent>(DL_BIND_EVENT_FN(Scene::OnMouseButtonPressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(DL_BIND_EVENT_FN(Scene::OnMouseButtonReleased));
    }

    void Scene::AddSubmesh(const Ref<Mesh>& mesh, uint32_t submeshIndex, const Ref<Material>& material, const Ref<Instance>& instance)
    {
        m_MeshRegistry.AddSubmesh(mesh, submeshIndex, material, instance);
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

        AddSubmesh(unitSphere, 0u, material, instance);
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

        AddSubmesh(unitSphere, 0u, material, instance);
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