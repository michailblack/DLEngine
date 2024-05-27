#include "dlpch.h"
#include "Camera.h"

#include "DLEngine/Core/Application.h"

namespace DLEngine
{
    Camera::Camera(float fov, float aspectRatio, float zNear, float zFar) noexcept
        : m_FOV(fov), m_AspectRatio(aspectRatio), m_Near(zNear), m_Far(zFar)
    {
    }

    void Camera::RotateForward(float angle) noexcept
    {
        m_Right = Math::Normalize(Math::RotateQuaternion(m_Right, m_Forward, angle));
        m_Up = Math::Normalize(Math::RotateQuaternion(m_Up, m_Forward, angle));
    }

    void Camera::RotateRight(float angle) noexcept
    {
        m_Up = Math::Normalize(Math::RotateQuaternion(m_Up, m_Right, angle));
        m_Forward = Math::Normalize(Math::RotateQuaternion(m_Forward, m_Right, angle));
    }

    void Camera::RotateUp(float angle) noexcept
    {
        m_Right = Math::Normalize(Math::RotateQuaternion(m_Right, m_Up, angle));
        m_Forward = Math::Normalize(Math::RotateQuaternion(m_Forward, m_Up, angle));
    }

    void Camera::RotateAxis(Math::Vec3 normalizedAxis, float angle) noexcept
    {
        m_Up = Math::Normalize(Math::RotateQuaternion(m_Up, normalizedAxis, angle));
        m_Right = Math::Normalize(Math::RotateQuaternion(m_Right, normalizedAxis, angle));
        m_Forward = Math::Normalize(Math::RotateQuaternion(m_Forward, normalizedAxis, angle));
    }

    Math::Ray Camera::ConstructRay(Math::Vec2 cursorPos) const noexcept
    {
        const auto& InvViewProjection = Math::Mat4x4::Inverse(GetViewMatrix() * GetProjectionMatrix());
        const auto& windowSize{ Application::Get().GetWindow()->GetSize() };

        Math::Vec4 BL = Math::Vec4{ -1.0f, -1.0f, 1.0f, 1.0f } * InvViewProjection;
        BL /= BL.w;

        Math::Vec4 BR = Math::Vec4{  1.0f, -1.0f, 1.0f, 1.0f } * InvViewProjection;
        BR /= BR.w;

        Math::Vec4 TL = Math::Vec4{ -1.0f,  1.0f, 1.0f, 1.0f } * InvViewProjection;
        TL /= TL.w;

        const Math::Vec4 Up = TL - BL;
        const Math::Vec4 Right = BR - BL;

        const Math::Vec4 P = BL + Right * (cursorPos.x / windowSize.x) + Up * (1.0f - cursorPos.y / windowSize.y);

        Math::Ray ray{};
        ray.Origin = P.xyz();
        ray.Direction = Math::Normalize(ray.Origin - m_Position);

        return ray;
    }

    Math::Mat4x4 Camera::GetProjectionMatrix() const noexcept
    {
        return Math::Mat4x4::Perspective(m_FOV, m_AspectRatio, m_Near, m_Far);
    }

    Math::Mat4x4 Camera::GetViewMatrix() const noexcept
    {
        return Math::Mat4x4::View(m_Right, m_Up, m_Forward, m_Position);
    }
}
