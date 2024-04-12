#include "dlpch.h"
#include "CameraController.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Input.h"

CameraController::CameraController(const Camera& camera)
    : m_Camera(camera)
{
}

void CameraController::OnUpdate(float dt)
{
    m_Transformed = false;

    if (Input::IsKeyPressed('W'))
    {
        m_Transformed = true;
        m_Camera.Translate(m_Camera.GetForward() * m_Velocity * dt);
    }

    if (Input::IsKeyPressed('S'))
    {
        m_Transformed = true;
        m_Camera.Translate(-m_Camera.GetForward() * m_Velocity * dt);
    }

    if (Input::IsKeyPressed('D'))
    {
        m_Transformed = true;
        m_Camera.Translate(m_Camera.GetRight() * m_Velocity * dt);
    }

    if (Input::IsKeyPressed('A'))
    {
        m_Transformed = true;
        m_Camera.Translate(-m_Camera.GetRight() * m_Velocity * dt);
    }

    if (Input::IsKeyPressed(VK_SPACE))
    {
        m_Transformed = true;
        m_Camera.Translate(m_Camera.GetUp() * m_Velocity * dt);
    }

    if (Input::IsKeyPressed(VK_CONTROL))
    {
        m_Transformed = true;
        m_Camera.Translate(-m_Camera.GetUp() * m_Velocity * dt);
    }

    if (Input::IsKeyPressed('Q'))
    {
        m_Transformed = true;
        m_Camera.RotateForward(-m_RotationVelocity * m_ForwardRotationVelocityScale * dt);
    }

    if (Input::IsKeyPressed('E'))
    {
        m_Transformed = true;
        m_Camera.RotateForward(m_RotationVelocity * m_ForwardRotationVelocityScale * dt);
    }

    if (m_IsRotating)
    {
        m_Transformed = true;

        const Math::Vec2 delta = Input::GetCursorPosition() - m_MouseStartPosition;
        const Math::Vec2 deltaDir = Math::Normalize(delta);

        const auto [wndWidth, wndHeight] = Application::Get().GetWindow()->GetSize();
        const float speed = Math::Length(delta) / (wndWidth * 0.5f) * m_RotationVelocity;

        m_Camera.RotateRight(-deltaDir.y * speed * dt);
        m_Camera.RotateUp(-deltaDir.x * speed * dt);
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
    m_Transformed = true;

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
    if (e.GetButton() == VK_LBUTTON)
    {
        m_MouseStartPosition = Input::GetCursorPosition();
        m_IsRotating = true;
    }

    return false;
}

bool CameraController::OnMouseButtonReleased(MouseButtonReleasedEvent& e)
{
    if (e.GetButton() == VK_LBUTTON)
    {
        m_IsRotating = false;
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

        std::cout << "Camera translation speed: " << m_Velocity << std::endl;
    }

    return false;
}
