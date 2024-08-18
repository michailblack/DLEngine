#pragma once
#include "DLEngine/Core/Base.h"

#include "DLEngine/Core/Events/ApplicationEvent.h"
#include "DLEngine/Core/Events/Event.h"
#include "DLEngine/Core/Events/KeyEvent.h"
#include "DLEngine/Core/Events/MouseEvent.h"

#include "DLEngine/Math/Math.h"
#include "DLEngine/Math/Vec2.h"

#include "DLEngine/Renderer/Camera.h"

namespace DLEngine
{
    class CameraController
    {
    public:
        using CameraResizeCallbackFn = std::function<void(Camera&, uint32_t, uint32_t)>;

    public:
        CameraController() noexcept = default;
        ~CameraController() = default;

        CameraController(const CameraController&) = delete;
        CameraController(CameraController&&) = delete;
        CameraController& operator=(const CameraController&) = delete;
        CameraController& operator=(CameraController&&) = delete;

        void OnUpdate(float dt);
        void OnEvent(Event& e);

        void SetCameraResizeCallback(const CameraResizeCallbackFn& callback) noexcept { m_CameraResizeCallbackFn = callback; }

        const Camera& GetCamera() const noexcept { return m_Camera; }

    private:
        bool OnWindowResize(WindowResizeEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnKeyReleased(KeyReleasedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);
        bool OnMouseScrolled(MouseScrolledEvent& e);

    private:
        Camera m_Camera;

        CameraResizeCallbackFn m_CameraResizeCallbackFn;

        float m_Velocity{ 0.003f };
        float m_VelocityScale{ 5.0f };
        float m_DeltaVelocityPercents{ 10.0f };
        float m_MinVelocity{ 0.0001f };
        float m_MaxVelocity{ 0.5f };

        // Radians per millisecond
        float m_RotationVelocity{ Math::Numeric::Pi * 1.e-3f };
        float m_ForwardRotationVelocityScale{ 0.25f };

        bool m_IsRotating{ false };

        Math::Vec2 m_MouseStartPosition{ 0.0f };
    };
}
