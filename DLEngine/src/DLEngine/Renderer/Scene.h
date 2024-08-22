#pragma once
#include "DLEngine/Core/Events/MouseEvent.h"

#include "DLEngine/Renderer/Mesh/MeshRegistry.h"

#include "DLEngine/Renderer/CameraController.h"
#include "DLEngine/Renderer/IDragger.h"

#include "DLEngine/Utils/DeltaTime.h"

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

    struct Environment
    {
        Ref<TextureCube> Skybox;
        Ref<TextureCube> DiffuseIrradianceMap;
        Ref<TextureCube> SpecularIrradianceMap;
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

        void AddSubmesh(const Ref<Mesh>& mesh, uint32_t submeshIndex, const Ref<Material>& material, const Ref<Instance>& instance);

        void AddDirectionalLight(const Math::Vec3& direction, const Math::Vec3& radiance, float solidAngle);

        void AddPointLight(const Math::Vec3& position, const Math::Vec3& irradiance, float radius, float distance, const Math::Vec3& emissionMeshTranslation);
        void AddPointLight(const Math::Vec3& position, const Math::Vec3& irradiance, float radius, float distance, const Ref<Instance>& meshInstance);

        void AddSpotLight(const Math::Vec3& position, const Math::Vec3& direction, float radius, float innerCutoffCos, float outerCutoffCos, const Math::Vec3& irradiance, float distance, const Math::Vec3& emissionMeshTranslation);
        void AddSpotLight(const Math::Vec3& position, const Math::Vec3& direction, float radius, float innerCutoffCos, float outerCutoffCos, const Math::Vec3& irradiance, float distance, const Ref<Instance>& meshInstance);

        const Camera& GetCamera() const { return m_SceneCameraController.GetCamera(); }

    private:
        bool OnWindowResize(WindowResizeEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);

    private:
        CameraController m_SceneCameraController;

        LightEnvironment m_LightEnvironment;

        MeshRegistry m_MeshRegistry;

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
        
        // Threshold is basically irradiance / radiance
        float SphereLightContributionDistance(float threshold, float radius);
    }
}