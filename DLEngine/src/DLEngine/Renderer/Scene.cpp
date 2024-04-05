#include "dlpch.h"
#include "Scene.h"

#include "DLEngine/Core/Input.h"
#include "DLEngine/Renderer/Renderer.h"

void Scene::Init()
{
}

void Scene::OnUpdate(float dt)
{
    ProcessInputs();

    if (m_ShouldDraw)
    {
        m_ShouldDraw = false;

        const Math::Vec3f moveDirection{ m_MoveDirection.Data[0], m_MoveDirection.Data[1], 0.0f };
        if (m_MouseWantsToMove)
        {
            m_MouseWantsToMove = false;
            m_Sphere.Center += moveDirection;
        }
        else
        {
            m_Sphere.Center += moveDirection * dt;
        }
        m_MoveDirection = { 0.0f, 0.0f };

        Renderer::Submit(m_Sphere);
        Renderer::SwapFramebuffer();
    }
}

void Scene::ProcessInputs()
{
    if (Input::IsMouseButtonPressed(Mouse::Button::Right))
    {
        const Math::Vec2<int32_t> mousePosition = Input::GetMousePosition();
    
        if (m_MouseStartedDragging)
        {
            const Math::Vec2f mouseWorldPos = Renderer::ScreenSpaceToWorldSpace(mousePosition);
            const Math::Vec2f prevMouseWorldPos = Renderer::ScreenSpaceToWorldSpace(m_PrevMousePosition);
            m_PrevMousePosition = mousePosition;
            Math::Vec2f delta = mouseWorldPos - prevMouseWorldPos;
    
            m_MoveDirection = { 0.0f, 0.0f };
    
            if (Math::Length(delta) > 0.0f)
            {
                delta.Data[1] *= -1.0f;
                m_MoveDirection = delta;
                m_ShouldDraw = true;
            }
    
            m_MouseWantsToMove = true;
        }
        else if (Renderer::MouseHoveringOverEntity(mousePosition) && Renderer::MouseHoveringOverEntity(m_PrevMousePosition))
        {
            m_MouseStartedDragging = true;
            m_PrevMousePosition = Input::GetMousePosition();
            m_MouseWantsToMove = true;
        }
    }
    else
    {
        m_MouseStartedDragging = false;
        m_PrevMousePosition = Input::GetMousePosition();
    
        if (Input::IsKeyPressed('W'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.Data[1] = m_EntitySpeed;
        }
    
        if (Input::IsKeyPressed('S'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.Data[1] = -m_EntitySpeed;
        }
    
        if (Input::IsKeyPressed('A'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.Data[0] = -m_EntitySpeed;
        }
    
        if (Input::IsKeyPressed('D'))
        {
            m_ShouldDraw = true;
            m_MoveDirection.Data[0] = m_EntitySpeed;
        }
    }
}
