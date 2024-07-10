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
        MouseScrolledEvent e{ 0 };
        OnMouseScrolled(e);
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

        if (Input::IsKeyPressed('E'))
        {
            m_Camera.Translate(m_Camera.GetUp() * m_Velocity * dt);
        }

        if (Input::IsKeyPressed('Q'))
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

        if (m_Dragger)
        {
            Math::Ray ray{};
            ray.Origin = m_Camera.ConstructFrustumPos(Input::GetCursorPosition());
            ray.Direction = Math::Normalize(m_Camera.ConstructFrustumPosRotOnly(Input::GetCursorPosition()));
            m_Dragger->Drag(ray);
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
        } break;
        case VK_RBUTTON:
        {
            IShadingGroup::IntersectInfo intersectInfo{};
            
            Math::Ray ray{};
            ray.Origin = m_Camera.ConstructFrustumPos(Input::GetCursorPosition());
            ray.Direction = Math::Normalize(m_Camera.ConstructFrustumPosRotOnly(Input::GetCursorPosition()));

            if (MeshSystem::Get().Intersects(ray, intersectInfo))
            {
                m_Dragger = CreateScope<MeshDragger>(MeshDragger{
                    intersectInfo.MeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint,
                    intersectInfo.MeshIntersectInfo.TriangleIntersectInfo.T,
                    intersectInfo.TransformIndex
                    });
            }
        } break;
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
        } break;
        case VK_RBUTTON:
        {
            m_Dragger.reset();
        } break;
        }

        return false;
    }

    bool CameraController::OnMouseScrolled(MouseScrolledEvent& e)
    {
        // To calculate the velocity, we use the sigmoid function,
        // which has domain (-1.0; 1.0) for considering floating point precision
        // and range (m_MinVelocity; m_MaxVelocity)

        static constexpr float eps{ 0.0001f };

        static constexpr float minX{ -1.0f };
        static constexpr float maxX{ 1.0f };

        static constexpr float sigmoidCenter{ (minX + maxX) * 0.5f };

        const float sigmoidScalingFactor{ 2.0f * Math::Log10((m_MaxVelocity - m_MinVelocity) / eps - 1.0f) / (maxX - minX) };

        const float step{ (maxX - minX) / m_VelocityFunStepRate };
        m_VelocityFunX = Math::Clamp(m_VelocityFunX + step * Math::Sign(static_cast<float>(e.GetOffset())), minX, maxX);

        m_Velocity = m_MinVelocity + (m_MaxVelocity - m_MinVelocity) / (1.0f + Math::Exp(-sigmoidScalingFactor * (m_VelocityFunX - sigmoidCenter)));

        return false;
    }
}
