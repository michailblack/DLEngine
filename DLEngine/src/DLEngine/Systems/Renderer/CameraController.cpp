#include "dlpch.h"
#include "CameraController.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Input.h"

#include "DLEngine/Systems/Mesh/MeshSystem.h"

namespace DLEngine
{
    CameraController::CameraController(const Camera& camera) noexcept
        : m_Camera(camera)
    {
    }

    void CameraController::OnUpdate(float dt)
    {
        // For offsetting m_EndDraggingRay.Origin when camera moves
        Math::Vec3 deltaCameraPos{ 0.0f };
        float deltaCameraRight{ 0.0f };
        float deltaCameraUp{ 0.0f };

        static constexpr Math::Vec3 worldUp{ 0.0f, 1.0f, 0.0f };

        if (Input::IsKeyPressed('W'))
        {
            m_Camera.Translate(m_Camera.GetForward() * m_Velocity * dt);
            deltaCameraPos += m_Camera.GetForward() * m_Velocity * dt;
        }

        if (Input::IsKeyPressed('S'))
        {
            m_Camera.Translate(-m_Camera.GetForward() * m_Velocity * dt);
            deltaCameraPos -= m_Camera.GetForward() * m_Velocity * dt;
        }

        if (Input::IsKeyPressed('D'))
        {
            m_Camera.Translate(m_Camera.GetRight() * m_Velocity * dt);
            deltaCameraPos += m_Camera.GetRight() * m_Velocity * dt;
        }

        if (Input::IsKeyPressed('A'))
        {
            m_Camera.Translate(-m_Camera.GetRight() * m_Velocity * dt);
            deltaCameraPos -= m_Camera.GetRight() * m_Velocity * dt;
        }

        if (Input::IsKeyPressed('Q'))
        {
            m_Camera.Translate(m_Camera.GetUp() * m_Velocity * dt);
            deltaCameraPos += m_Camera.GetUp() * m_Velocity * dt;
        }

        if (Input::IsKeyPressed('E'))
        {
            m_Camera.Translate(-m_Camera.GetUp() * m_Velocity * dt);
            deltaCameraPos -= m_Camera.GetUp() * m_Velocity * dt;
        }

        if (m_IsRotating)
        {
            const Math::Vec2 delta = Input::GetCursorPosition() - m_MouseStartPosition;
            const Math::Vec2 deltaDir = Math::Normalize(delta);

            const auto [wndWidth, wndHeight] = Application::Get().GetWindow()->GetSize();
            const float speed = Math::Length(delta) / (wndWidth * 0.5f) * m_RotationVelocity;

            deltaCameraRight = deltaDir.y * speed * dt;
            m_Camera.RotateRight(-deltaCameraRight);

            deltaCameraUp = deltaDir.x * speed * dt;
            m_Camera.RotateAxis(worldUp, -deltaCameraUp);
        }

        if (m_WantsToDrag)
        {
            if (!m_Dragger)
            {
                IShadingGroup::IntersectInfo intersectInfo{};
                if (MeshSystem::Get().Intersects(m_StartDraggingRay, intersectInfo))
                {
                    m_Dragger = CreateScope<MeshDragger>(MeshDragger{
                        intersectInfo.MeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint,
                        intersectInfo.MeshIntersectInfo.TriangleIntersectInfo.T,
                        intersectInfo.TransformIndex
                    });
                }
            }

            if (m_Dragger)
            {
                m_EndDraggingRay = m_Camera.ConstructRay(Input::GetCursorPosition());
                
                m_EndDraggingRay.Origin -= deltaCameraPos;
                m_EndDraggingRay.Direction = Math::Normalize(
                    Math::RotateQuaternion(m_EndDraggingRay.Direction, m_Camera.GetRight(), deltaCameraRight)
                );
                m_EndDraggingRay.Direction = Math::Normalize(
                    Math::RotateQuaternion(m_EndDraggingRay.Direction, worldUp, deltaCameraUp)
                );

                m_Dragger->Drag(m_EndDraggingRay);
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
