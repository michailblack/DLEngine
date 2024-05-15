#pragma once
#include "DLEngine/Math/Mat4x4.h"
#include "DLEngine/Math/Primitives.h"
#include "DLEngine/Math/Vec2.h"
#include "DLEngine/Math/Vec3.h"

namespace DLEngine
{
    class Camera
    {
    public:
        Camera(float fov, float aspectRatio, float zNear, float zFar) noexcept;

        void SetAspectRatio(float aspectRatio) noexcept { m_AspectRatio = aspectRatio; }

        void Translate(const Math::Vec3& translation) noexcept { m_Position += translation; }

        void RotateForward(float angle) noexcept;
        void RotateRight(float angle) noexcept;
        void RotateUp(float angle) noexcept;
        void RotateAxis(Math::Vec3 normalizedAxis, float angle) noexcept;

        Math::Ray ConstructRay(Math::Vec2 cursorPos) const noexcept;

        Math::Vec3 GetPosition() const noexcept { return m_Position; }

        Math::Vec3 GetRight() const noexcept { return m_Right; }
        Math::Vec3 GetUp() const noexcept { return m_Up; }
        Math::Vec3 GetForward() const noexcept { return m_Forward; }

        Math::Mat4x4 GetProjectionMatrix() const noexcept;
        Math::Mat4x4 GetViewMatrix() const noexcept;

    private:
        Math::Vec3 m_Position{ 0.0f, 0.0f, 0.0f };

        Math::Vec3 m_Right{ 1.0f, 0.0f, 0.0f };
        Math::Vec3 m_Up{ 0.0f, 1.0f, 0.0f };
        Math::Vec3 m_Forward{ 0.0f, 0.0f, 1.0f };

        float m_FOV;
        float m_AspectRatio;
        float m_Near;
        float m_Far;
    };
}
