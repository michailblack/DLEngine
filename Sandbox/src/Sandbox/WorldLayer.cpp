#include "WorldLayer.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Input.h"
#include "DLEngine/Renderer/Renderer.h"

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
    ProcessInputs();

    if (m_ShouldDraw)
    {
        m_ShouldDraw = false;

        const Math::Vec3f moveDirection{ m_MoveDirection.Data[0], m_MoveDirection.Data[1], 0.0f };
        m_MoveDirection = { 0.0f, 0.0f };
        m_Sphere.Center += moveDirection * dt;

        Renderer::Draw(m_Sphere);
    }
}

void WorldLayer::ProcessInputs()
{
    if (Input::Get().IsKeyPressed(VK_RBUTTON))
    {
        const Math::Vec2<int32_t> mousePosition = Input::Get().GetCursorPosition();
        const Math::Vec2f mouseWorldPos = Renderer::ScreenSpaceToWorldSpace(mousePosition);

        if (!m_MouseStartedDragging)
        {
            m_MouseStartedDragging = true;
            m_ShouldDraw = true;
            m_PrevMousePosition = Input::Get().GetCursorPosition();
            m_Sphere.Center = { mouseWorldPos.Data[0], -mouseWorldPos.Data[1], m_Sphere.Center.Data[2] };
            return;
        }

        const Math::Vec2f prevMouseWorldPos = Renderer::ScreenSpaceToWorldSpace(m_PrevMousePosition);
        m_PrevMousePosition = mousePosition;
        Math::Vec2f delta = mouseWorldPos - prevMouseWorldPos;

        if (Math::Length(delta) > 0.0f)
        {
            delta.Data[1] *= -1.0f;
            m_Sphere.Center = { mouseWorldPos.Data[0], -mouseWorldPos.Data[1], m_Sphere.Center.Data[2] };
            m_ShouldDraw = true;
        }
    }
    else
    {
        m_MouseStartedDragging = false;
        m_PrevMousePosition = Input::Get().GetCursorPosition();
    
        if (Input::Get().IsKeyPressed('W'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.Data[1] = m_EntitySpeed;
        }
    
        if (Input::Get().IsKeyPressed('S'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.Data[1] = -m_EntitySpeed;
        }
    
        if (Input::Get().IsKeyPressed('A'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.Data[0] = -m_EntitySpeed;
        }
    
        if (Input::Get().IsKeyPressed('D'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.Data[0] = m_EntitySpeed;
        }
    }
}
