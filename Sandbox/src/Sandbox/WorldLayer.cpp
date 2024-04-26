#include "WorldLayer.h"

#include "DLEngine/Math/Intersections.h"

#include "DLEngine/Renderer/RenderCommand.h"
#include "DLEngine/Renderer/Renderer.h"

WorldLayer::WorldLayer()
    : m_CameraController(Camera { Math::ToRadians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f })
{
}

WorldLayer::~WorldLayer()
{
}

void WorldLayer::OnAttach()
{
    PlaneInstance planeInst;
    planeInst.Plane.Origin = Math::Vec3 { 0.0f, 0.0f, 0.0f };
    planeInst.Plane.Normal = Math::Vec3 { 0.0f, 1.0f, 0.0f };
    planeInst.Mat.SetAlbedo(Math::Vec3 { 0.1f });
    planeInst.Mat.SetSpecularPower(32.0f);
    m_Planes.push_back(CreateRef<PlaneInstance>(planeInst));

    SphereInstance sphereInstance;
    sphereInstance.Sphere.Center = Math::Vec3 { 2.0f, 2.0f, 2.0f };
    sphereInstance.Sphere.Radius = 1.0f;
    sphereInstance.Mat.SetAlbedo(Math::Vec3 { 0.5f, 0.1f, 0.3f });
    sphereInstance.Mat.SetSpecularPower(64.0f);
    m_Spheres.push_back(CreateRef<SphereInstance>(sphereInstance));

    sphereInstance.Sphere.Center = Math::Vec3 { -1.0f, 1.0f, 3.0f };
    sphereInstance.Sphere.Radius = 0.5f;
    sphereInstance.Mat.SetAlbedo(Math::Vec3 { 0.0f, 0.2f, 0.01f });
    sphereInstance.Mat.SetSpecularPower(128.0f);
    m_Spheres.push_back(CreateRef<SphereInstance>(sphereInstance));

    MeshInstance cubeInstance;
    cubeInstance.Transform = Math::Mat4x4::Translate(Math::Vec3 { -3.0f, 3.0f, 1.0f });
    cubeInstance.InvTransform = Math::Mat4x4::Inverse(cubeInstance.Transform);
    cubeInstance.Mat.SetAlbedo(Math::Vec3 { 0.987f, 0.705f, 0.11f });
    cubeInstance.Mat.SetSpecularPower(16.0f);
    m_Cubes.push_back(CreateRef<MeshInstance>(cubeInstance));

    cubeInstance.Transform = Math::Mat4x4::Scale(Math::Vec3 { 0.5f }) * Math::Mat4x4::Translate(Math::Vec3 { 0.5f, 2.0f, 2.0f });
    cubeInstance.InvTransform = Math::Mat4x4::Inverse(cubeInstance.Transform);
    cubeInstance.Mat.SetAlbedo(Math::Vec3 { 0.79f, 0.2f, 0.19f });
    cubeInstance.Mat.SetSpecularPower(256.0f);
    m_Cubes.push_back(CreateRef<MeshInstance>(cubeInstance));

    m_Environment = CreateRef<Environment>();

    constexpr float lightIntensity { 20.0f };

    m_Environment->IndirectLightingColor = Math::Vec3 { 0.5f };

    m_Environment->Sun.Direction = Math::Normalize(Math::Vec3 { 1.0f, -1.0f, 1.0f });
    m_Environment->Sun.Color = Math::Vec3 { 0.99f, 0.955f, 0.564f } * lightIntensity / 2.0f;

    PointLight pointLight;
    pointLight.Position = Math::Vec3 { -1.0f, 2.0f, 2.0f };
    pointLight.Color = Math::Vec3 { 0.0f, 0.24f, 0.91f } * lightIntensity;
    pointLight.Linear = 0.14f;
    pointLight.Quadratic = 0.07f;
    m_Environment->PointLights.push_back(pointLight);

    pointLight.Position = Math::Vec3 { -5.0f, 3.0f, 2.0f };
    pointLight.Color = Math::Vec3 { 0.89f, 0.04f, 0.04f } * lightIntensity;
    pointLight.Linear = 0.35f;
    pointLight.Quadratic = 0.44f;
    m_Environment->PointLights.push_back(pointLight);

    SpotLight spotLight;
    spotLight.Position = Math::Vec3 { 0.0f, 3.0f, 0.0f };
    spotLight.Color = Math::Vec3 { 0.0f, 0.0f, 1.0f } * lightIntensity;
    spotLight.Direction = Math::Normalize(Math::Vec3 { 0.0f, -1.0f, 0.0f });
    spotLight.InnerCutoffCos = Math::Cos(Math::ToRadians(12.5f));
    spotLight.OuterCutoffCos = Math::Cos(Math::ToRadians(17.5f));
    spotLight.Linear = 0.07f;
    spotLight.Quadratic = 0.017f;
    m_Environment->SpotLights.push_back(spotLight);

    m_Environment->Exposure = 1.0f;
}

void WorldLayer::OnDetach()
{
    
}

void WorldLayer::OnUpdate(float dt)
{
    /*if (m_CameraController.AskedForDragger())
        m_CameraController.SetDragger(FindDragger(m_CameraController.GetDraggingRay()));
    
    if (m_CameraController.IsCameraTransformed() || m_ShouldRender)
    {
        m_ShouldRender = false;
    
        Renderer::BeginScene(m_CameraController.GetCamera(), m_Environment);
    
        for (const auto& sphere : m_Spheres)
            Renderer::Submit(sphere);
    
        for (const auto& plane : m_Planes)
            Renderer::Submit(plane);
    
        for (const auto& cube : m_Cubes)
            Renderer::Submit(cube);
    
        Renderer::EndScene();
    }
    
    m_CameraController.OnUpdate(dt);*/

    DrawTestTriangle();
}

void WorldLayer::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<AppRenderEvent>(DL_BIND_EVENT_FN(WorldLayer::OnAppRenderEvent));

    m_CameraController.OnEvent(e);
}

bool WorldLayer::OnAppRenderEvent(AppRenderEvent& e)
{
    m_ShouldRender = true;
    return false;
}

Scope<IDragger> WorldLayer::FindDragger(const Math::Ray& ray) const
{
    Math::IntersectInfo intersectInfo {};
    const Math::Plane nearPlane { ray.Origin, m_CameraController.GetCamera().GetForward() };

    Scope<IDragger> dragger {};

    for (const auto& sphere : m_Spheres)
    {
        if (Math::Intersects(ray, sphere->Sphere, intersectInfo))
        {
            const float distanceToDraggingPlane { Math::Distance(intersectInfo.IntersectionPoint, nearPlane) };
            dragger.reset(new ISphereDragger { sphere, intersectInfo.IntersectionPoint, distanceToDraggingPlane });
        }
    }

    for (const auto& plane : m_Planes)
    {
        if (Math::Intersects(ray, plane->Plane, intersectInfo))
        {
            const float distanceToDraggingPlane { Math::Distance(intersectInfo.IntersectionPoint, nearPlane) };
            dragger.reset(new IPlaneDragger { plane, intersectInfo.IntersectionPoint, distanceToDraggingPlane });
        }
    }

    for (const auto& cube : m_Cubes)
    {
        if (Math::Intersects(ray, *cube, Mesh::GetUnitCube(), intersectInfo))
        {
            const float distanceToDraggingPlane { Math::Distance(intersectInfo.IntersectionPoint, nearPlane) };
            dragger.reset(new IMeshDragger { cube, intersectInfo.IntersectionPoint, distanceToDraggingPlane });
        }
    }

    return dragger;
}

void WorldLayer::DrawTestTriangle()
{
    const auto& renderTargetView { Application::Get().GetWindow()->GetRenderTargetView() };
    RenderCommand::BindRenderTargetView(renderTargetView);
    RenderCommand::Clear(renderTargetView, Math::Vec4 { 0.1f, 0.1f, 0.1f, 1.0f });
}
