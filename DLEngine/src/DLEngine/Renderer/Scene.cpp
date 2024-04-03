#include "dlpch.h"
#include "Scene.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Events/EventBus.h"
#include "DLEngine/Renderer/Renderer.h"

void Scene::Init()
{
    auto& eventBus = EventBus::Get();
    eventBus.Subscribe<KeyPressedEvent>(DL_BIND_EVENT_CALLBACK(OnKeyPressedEvent));
    eventBus.Subscribe<MouseMovedEvent>(DL_BIND_EVENT_CALLBACK(OnMouseMovedEvent));
    eventBus.Subscribe<MouseButtonPressedEvent>(DL_BIND_EVENT_CALLBACK(OnMouseButtonPressedEvent));
    eventBus.Subscribe<MouseButtonReleasedEvent>(DL_BIND_EVENT_CALLBACK(OnMouseButtonReleasedEvent));

    m_Entities.reserve(10);
    m_Entities.push_back({ { 0.0f, 0.0f, 5.0f }, 1.0f });
}

void Scene::OnUpdate(float dt)
{
    if (Math::Length(m_MoveDirection) > 0.0f)
    {
        for (auto& entity : m_Entities)
        {
            Math::Vec3f moveDirection { m_MoveDirection.Data[0], m_MoveDirection.Data[1], 0.0f };
            entity.Center += moveDirection * dt;
        }

        m_MoveDirection = { 0.0f, 0.0f };
    }

    Renderer::Clear({ 0, 0, 0 });
    
    for (const auto& entity : m_Entities)
        Renderer::Submit(entity);

    Renderer::SwapFramebuffer();
}

void Scene::OnKeyPressedEvent(const KeyPressedEvent& event)
{

    switch (event.GetKeyCode())
    {
        case 'W':
            m_MoveDirection.Data[1] +=  0.01f;
            break;
        case 'A':
            m_MoveDirection.Data[0] += -0.01f;
            break;
        case 'S':
            m_MoveDirection.Data[1] += -0.01f;
            break;
        case 'D':
            m_MoveDirection.Data[0] +=  0.01f;
            break;
        default:
            break;
    }
}

void Scene::OnMouseMovedEvent(const MouseMovedEvent& event)
{
    if (m_CanMove)
    {
        const Math::Vec2<int32_t>& mousePosition = { event.GetX(), event.GetY() };
        Math::Vec2<int32_t> delta = mousePosition - m_MousePosition;
        delta.Data[0] *= -1;
        m_MousePosition = mousePosition;
        m_MoveDirection = static_cast<Math::Vec2f>(delta) * -0.001f;
    }
}

void Scene::OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event)
{
    if (event.GetButton() == MouseButton::Right)
    {
        m_MousePosition.Data[0] = event.GetX();
        m_MousePosition.Data[1] = event.GetY();

        m_CanMove = true;
    }
}

void Scene::OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event)
{
    if (event.GetButton() == MouseButton::Right)
    {
        m_MousePosition.Data[0] = event.GetX();
        m_MousePosition.Data[1] = event.GetY();

        m_CanMove = false;
    }
}
