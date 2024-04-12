#include "WorldLayer.h"

#include <iostream>

#include "DLEngine/Core/Application.h"

#include "DLEngine/Math/Math.h"

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
    
}

void WorldLayer::OnDetach()
{
    
}

void WorldLayer::OnUpdate(float dt)
{
    if (m_CameraController.IsCameraTransformed())
    {
        Renderer::BeginScene(m_CameraController.GetCamera());

        Renderer::Submit(m_Sphere);

        Renderer::EndScene();
    }

    m_CameraController.OnUpdate(dt);
}

void WorldLayer::OnEvent(Event& e)
{
    m_CameraController.OnEvent(e);
}
