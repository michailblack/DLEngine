#include "WorldLayer.h"

#include <iostream>

#include "DLEngine/Core/Application.h"
#include "DLEngine/Math/Intersections.h"

#include "DLEngine/Math/Math.h"

#include "DLEngine/Renderer/Renderer.h"
#include "DLEngine/Renderer/ThreadPool.h"

WorldLayer::WorldLayer()
    : m_CameraController(Camera { Math::ToRadians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f })
{
}

WorldLayer::~WorldLayer()
{
}

void WorldLayer::OnAttach()
{
    m_Spheres.push_back(CreateRef<SphereInstance>(SphereInstance { Math::Vec3 { 2.0f, 2.0f, 2.0f }, 1.0f }));
    m_Spheres.push_back(CreateRef<SphereInstance>(SphereInstance { Math::Vec3 {-1.0f, 1.0f, 3.0f }, 0.5f }));

    m_Planes.push_back(CreateRef<PlaneInstance>(PlaneInstance { Math::Vec3 { 0.0f, 0.0f, 0.0f }, Math::Vec3 { 0.0f, 1.0f, 0.0f } }));

    MeshInstance cube;
    cube.Transform = Math::Mat4x4::Translate(Math::Vec3 { -3.0f, 3.0f, 1.0f });
    cube.InvTransform = Math::Mat4x4::Inverse(cube.Transform);
    m_Cubes.push_back(CreateRef<MeshInstance>(cube));

    cube.Transform = Math::Mat4x4::Scale(Math::Vec3 { 0.5f }) * Math::Mat4x4::Translate(Math::Vec3 { 0.0f, 1.5f, 2.0f });
    cube.InvTransform = Math::Mat4x4::Inverse(cube.Transform);
    m_Cubes.push_back(CreateRef<MeshInstance>(cube));
}

void WorldLayer::OnDetach()
{
    
}

void WorldLayer::OnUpdate(float dt)
{
    //std::cout << "dt: " << dt << "ms\n";

    if (m_CameraController.IsCameraTransformed())
    {
        Renderer::BeginScene(m_CameraController.GetCamera());

        for (const auto& sphere : m_Spheres)
            Renderer::Submit(sphere);

        for (const auto& plane : m_Planes)
            Renderer::Submit(plane);

        for (const auto& cube : m_Cubes)
            Renderer::Submit(cube);

        Renderer::EndScene();
    }

    m_CameraController.OnUpdate(dt);
}

void WorldLayer::OnEvent(Event& e)
{
    m_CameraController.OnEvent(e);
}
