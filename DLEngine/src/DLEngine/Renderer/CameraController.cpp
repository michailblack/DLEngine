#include "dlpch.h"
#include "CameraController.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Input.h"

#include "DLEngine/Mesh/MeshSystem.h"

#include "DLEngine/Renderer/Renderer.h"

namespace DLEngine
{
    CameraController::CameraController(const Camera& camera) noexcept
        : m_Camera(camera)
    {
    }

    void CameraController::OnUpdate(float dt)
    {
        if (Input::IsKeyPressed('W'))
        {
            m_Camera.Translate(m_Camera.GetForward() * m_Velocity * dt);
        }

        if (Input::IsKeyPressed('S'))
        {
            m_Camera.Translate(-m_Camera.GetForward() * m_Velocity * dt);
        }

        if (Input::IsKeyPressed('D'))
        {
            m_Camera.Translate(m_Camera.GetRight() * m_Velocity * dt);
        }

        if (Input::IsKeyPressed('A'))
        {
            m_Camera.Translate(-m_Camera.GetRight() * m_Velocity * dt);
        }

        if (Input::IsKeyPressed('Q'))
        {
            m_Camera.Translate(m_Camera.GetUp() * m_Velocity * dt);
        }

        if (Input::IsKeyPressed('E'))
        {
            m_Camera.Translate(-m_Camera.GetUp() * m_Velocity * dt);
        }

        if (m_IsRotating)
        {
            const Math::Vec2 delta = Input::GetCursorPosition() - m_MouseStartPosition;
            const Math::Vec2 deltaDir = Math::Normalize(delta);

            const auto [wndWidth, wndHeight] = Application::Get().GetWindow()->GetSize();
            const float speed = Math::Length(delta) / (wndWidth * 0.5f) * m_RotationVelocity;

            static constexpr Math::Vec3 worldUp{ 0.0f, 1.0f, 0.0f };

            m_Camera.RotateRight(-deltaDir.y * speed * dt);
            m_Camera.RotateAxis(worldUp, -deltaDir.x * speed * dt);
        }

        if (m_WantsToDrag)
        {
            const Math::Plane nearPlane{ m_StartDraggingRay.Origin, m_Camera.GetForward() };

            /*if (!m_Dragger)
                MeshSystem::Intersects(m_StartDraggingRay, m_Camera.GetForward(), m_Dragger);*/

            if (m_Dragger)
            {
                m_EndDraggingRay = m_Camera.ConstructRay(Input::GetCursorPosition());

                m_Dragger->Drag(nearPlane, m_EndDraggingRay);

                m_StartDraggingRay = m_EndDraggingRay;
            }
        }
    }

    void CameraController::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>(DL_BIND_EVENT_FN(CameraController::OnWindowResize));
        dispatcher.Dispatch<KeyPressedEvent>(DL_BIND_EVENT_FN(CameraController::OnKeyPressed));
        dispatcher.Dispatch<KeyReleasedEvent>(DL_BIND_EVENT_FN(CameraController::OnKeyReleased));
        dispatcher.Dispatch<MouseButtonPressedEvent>(DL_BIND_EVENT_FN(CameraController::OnMouseButtonPressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(DL_BIND_EVENT_FN(CameraController::OnMouseButtonReleased));
        dispatcher.Dispatch<MouseScrolledEvent>(DL_BIND_EVENT_FN(CameraController::OnMouseScrolled));
    }

    bool CameraController::OnWindowResize(WindowResizeEvent& e)
    {
        m_Camera.SetAspectRatio(static_cast<float>(e.GetWidth()) / static_cast<float>(e.GetHeight()));

        return false;
    }

    bool CameraController::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetKeyCode() == VK_SHIFT)
        {
            m_Velocity *= m_VelocityScale;
        }

        return false;
    }

    bool CameraController::OnKeyReleased(KeyReleasedEvent& e)
    {
        if (e.GetKeyCode() == VK_SHIFT)
        {
            m_Velocity /= m_VelocityScale;
        }

        return false;
    }

    bool CameraController::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        switch (e.GetButton())
        {
        case VK_LBUTTON:
        {
            m_MouseStartPosition = Input::GetCursorPosition();
            m_IsRotating = true;
            break;
        }
        case VK_RBUTTON:
        {
            m_StartDraggingRay = m_Camera.ConstructRay(Input::GetCursorPosition());
            m_EndDraggingRay = m_StartDraggingRay;
            m_WantsToDrag = true;
            break;
        }
        }

        return false;
    }

    bool CameraController::OnMouseButtonReleased(MouseButtonReleasedEvent& e)
    {
        switch (e.GetButton())
        {
        case VK_LBUTTON:
        {
            m_IsRotating = false;
            break;
        }
        case VK_RBUTTON:
        {
            m_StartDraggingRay = Math::Ray{};
            m_EndDraggingRay = Math::Ray{};
            m_Dragger.reset();
            m_WantsToDrag = false;
            break;
        }
        }

        return false;
    }

    bool CameraController::OnMouseScrolled(MouseScrolledEvent& e)
    {
        if (!Input::IsKeyPressed(VK_SHIFT))
        {
            if (e.GetOffset() > 0)
            {
                m_Velocity *= 1.0f + m_DeltaVelocityPercents / 100.0f;
                if (m_Velocity > m_MaxVelocity)
                {
                    m_Velocity = m_MaxVelocity;
                }
            }
            else
            {
                m_Velocity *= 1.0f - m_DeltaVelocityPercents / 100.0f;
                if (m_Velocity < m_MinVelocity)
                {
                    m_Velocity = m_MinVelocity;
                }
            }
        }

        return false;
    }
}
