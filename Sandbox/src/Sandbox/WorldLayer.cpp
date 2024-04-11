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

        const Math::Vec3 moveDirection{ m_MoveDirection.x, m_MoveDirection.y, 0.0f };
        m_MoveDirection = { 0.0f, 0.0f };
        m_Sphere.Center = m_Sphere.Center + moveDirection * dt;

        Renderer::Draw(m_Sphere);
    }
}

void WorldLayer::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowResizeEvent>(DL_BIND_EVENT_FN(WorldLayer::OnWindowResize));
}

void WorldLayer::ProcessInputs()
{
    if (Input::Get().IsKeyPressed(VK_RBUTTON))
    {
        const Math::Vec2 mousePosition = Input::Get().GetCursorPosition();
        const Math::Vec2 mouseWorldPos = Renderer::ScreenSpaceToWorldSpace(mousePosition);

        if (!m_MouseStartedDragging)
        {
            m_MouseStartedDragging = true;
            m_ShouldDraw = true;
            m_PrevMousePosition = Input::Get().GetCursorPosition();
            m_Sphere.Center = { mouseWorldPos.x, -mouseWorldPos.y, m_Sphere.Center.z };
            return;
        }

        const Math::Vec2 prevMouseWorldPos = Renderer::ScreenSpaceToWorldSpace(m_PrevMousePosition);
        m_PrevMousePosition = mousePosition;
        Math::Vec2 delta = mouseWorldPos - prevMouseWorldPos;

        if (Math::Length(delta) > 0.0f)
        {
            delta.y *= -1.0f;
            m_Sphere.Center = { mouseWorldPos.x, -mouseWorldPos.y, m_Sphere.Center.z };
            m_ShouldDraw = true;
        }
    }
    else
    {
        m_MouseStartedDragging = false;
        m_PrevMousePosition = Input::Get().GetCursorPosition();

        if (Input::Get().IsKeyPressed('S'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.y = -m_EntitySpeed;
        }

        if (Input::Get().IsKeyPressed('W'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.y = m_EntitySpeed;
        }
    
        if (Input::Get().IsKeyPressed('A'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.x = -m_EntitySpeed;
        }
    
        if (Input::Get().IsKeyPressed('D'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.x = m_EntitySpeed;
        }
    }
}

bool WorldLayer::OnWindowResize(WindowResizeEvent& event)
{
    m_ShouldDraw = true;
    return false;
}
