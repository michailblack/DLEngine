#pragma once
#include <numbers>

#include "DLEngine/Core/Events/ApplicationEvent.h"
#include "DLEngine/Core/Events/Event.h"
#include "DLEngine/Core/Events/KeyEvent.h"
#include "DLEngine/Core/Events/MouseEvent.h"

#include "DLEngine/Renderer/Camera.h"

#include "DLEngine/Math/Vec2.h"

class CameraController
{
public:
    CameraController(const Camera& camera);
    ~CameraController() = default;

    CameraController(const CameraController&) = delete;
    CameraController(CameraController&&) = delete;
    CameraController& operator=(const CameraController&) = delete;
    CameraController& operator=(CameraController&&) = delete;

    void OnUpdate(float dt);
    void OnEvent(Event& e);

    bool IsCameraTransformed() const { return m_Transformed; }

    const Camera& GetCamera() const { return m_Camera; }

private:
    bool OnWindowResize(WindowResizeEvent& e);
    bool OnKeyPressed(KeyPressedEvent& e);
    bool OnKeyReleased(KeyReleasedEvent& e);
    bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
    bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);
    bool OnMouseScrolled(MouseScrolledEvent& e);

private:
    Camera m_Camera;

    float m_Velocity { 0.003f };
    float m_VelocityScale { 5.0f };
    float m_DeltaVelocityPercents { 5.0f };
    float m_MinVelocity { 0.001f };
    float m_MaxVelocity { 0.005f };

    // Radians per millisecond
    float m_RotationVelocity { std::numbers::pi_v<float> * 1.e-3f };
    float m_ForwardRotationVelocityScale { 0.25f };

    bool m_IsRotating { false };

    Math::Vec2 m_MouseStartPosition { 0.0f, 0.0f };

    bool m_Transformed { true };
};
