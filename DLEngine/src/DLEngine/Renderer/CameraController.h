#pragma once
#include <numbers>

#include "DLEngine/Core/Events/ApplicationEvent.h"
#include "DLEngine/Core/Events/Event.h"
#include "DLEngine/Core/Events/KeyEvent.h"
#include "DLEngine/Core/Events/MouseEvent.h"

#include "DLEngine/Math/Primitives.h"
#include "DLEngine/Math/Vec2.h"

#include "DLEngine/Renderer/Camera.h"
#include "DLEngine/Renderer/IDragger.h"

namespace DLEngine
{
    class CameraController
    {
    public:
        CameraController(const Camera& camera) noexcept;
        ~CameraController() = default;

        CameraController(const CameraController&) = delete;
        CameraController(CameraController&&) = delete;
        CameraController& operator=(const CameraController&) = delete;
        CameraController& operator=(CameraController&&) = delete;

        void OnUpdate(float dt);
        void OnEvent(Event& e);

        bool AskedForDragger() const noexcept { return m_AskForDragger; }
        void SetDragger(Scope<IDragger> dragger) noexcept { m_Dragger = std::move(dragger); m_AskForDragger = false; }

        const Camera& GetCamera() const noexcept { return m_Camera; }
        const Math::Ray& GetDraggingRay() const noexcept { return m_StartDraggingRay; }

    private:
        bool OnWindowResize(WindowResizeEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnKeyReleased(KeyReleasedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);
        bool OnMouseScrolled(MouseScrolledEvent& e);

    private:
        Camera m_Camera;

        float m_Velocity{ 0.003f };
        float m_VelocityScale{ 5.0f };
        float m_DeltaVelocityPercents{ 5.0f };
        float m_MinVelocity{ 0.001f };
        float m_MaxVelocity{ 0.01f };

        // Radians per millisecond
        float m_RotationVelocity{ std::numbers::pi_v<float> *1.e-3f };
        float m_ForwardRotationVelocityScale{ 0.25f };

        bool m_IsRotating{ false };

        Math::Vec2 m_MouseStartPosition{ 0.0f };

        Math::Ray m_StartDraggingRay{};
        Math::Ray m_EndDraggingRay{};
        Scope<IDragger> m_Dragger{};
        bool m_AskForDragger{ false };
    };
}
