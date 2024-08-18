#include "dlpch.h"
#include "Camera.h"

#include "DLEngine/Core/Application.h"

namespace DLEngine
{

    void Camera::SetPerspectiveProjectionFov(float fovAngleY, float aspectRatio, float nearZ, float farZ) noexcept
    {
        m_FovAngleY = fovAngleY;
        m_AspectRatio = aspectRatio;
        m_NearZ = nearZ;
        m_FarZ = farZ;

        m_ProjectionType = ProjectionType::PerspectiveFOV;
    }

    void Camera::SetPerspectiveProjection(float viewWidth, float viewHeight, float nearZ, float farZ) noexcept
    {
        m_ViewWidth = viewWidth;
        m_ViewHeight = viewHeight;
        m_NearZ = nearZ;
        m_FarZ = farZ;

        m_ProjectionType = ProjectionType::Perspective;
    }

    void Camera::SetOrthographicProjection(float viewWidth, float viewHeight, float nearZ, float farZ) noexcept
    {
        m_ViewWidth = viewWidth;
        m_ViewHeight = viewHeight;
        m_NearZ = nearZ;
        m_FarZ = farZ;

        m_ProjectionType = ProjectionType::Orthographic;
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
    
    Math::Mat4x4 Camera::GetProjectionMatrix() const noexcept
    {
        switch (m_ProjectionType)
        {
        case Camera::ProjectionType::PerspectiveFOV: return Math::Mat4x4::PerspectiveFov(m_FovAngleY, m_AspectRatio, m_NearZ, m_FarZ);
        case Camera::ProjectionType::Perspective:    return Math::Mat4x4::Perspective(m_ViewWidth, m_ViewHeight, m_NearZ, m_FarZ);
        case Camera::ProjectionType::Orthographic:   return Math::Mat4x4::Orthographic(m_ViewWidth, m_ViewHeight, m_NearZ, m_FarZ);
        case Camera::ProjectionType::None:
        default:
            DL_ASSERT(false, "Projection type for camera is not set!");
            return Math::Mat4x4::Identity();
        }
    }

    Math::Mat4x4 Camera::GetViewMatrix() const noexcept
    {
        return Math::Mat4x4::LookTo(m_Position, m_Forward, m_Up);
    }

    Math::Vec3 Camera::ConstructFrustumPos(Math::Vec2 cursorPos) const noexcept
    {
        const auto& InvViewProjection = Math::Mat4x4::Inverse(GetViewMatrix() * GetProjectionMatrix());
        const auto& windowSize{ Application::Get().GetWindow().GetSize() };

        Math::Vec4 BL = Math::Vec4{ -1.0f, -1.0f, 1.0f, 1.0f } * InvViewProjection;
        BL /= BL.w;

        Math::Vec4 BR = Math::Vec4{  1.0f, -1.0f, 1.0f, 1.0f } * InvViewProjection;
        BR /= BR.w;

        Math::Vec4 TL = Math::Vec4{ -1.0f,  1.0f, 1.0f, 1.0f } * InvViewProjection;
        TL /= TL.w;

        const Math::Vec4 Up = TL - BL;
        const Math::Vec4 Right = BR - BL;

        const Math::Vec4 P = BL + Right * (cursorPos.x / windowSize.x) + Up * (1.0f - cursorPos.y / windowSize.y);

        return Math::Vec3{ P.x, P.y, P.z };
    }

    Math::Vec3 Camera::ConstructFrustumPosRotOnly(Math::Vec2 cursorPos) const noexcept
    {
        auto viewMatrix{ GetViewMatrix() };

        // Getting rid of translation
        viewMatrix._41 = 0.0f;
        viewMatrix._42 = 0.0f;
        viewMatrix._43 = 0.0f;

        const auto& InvViewProjection = Math::Mat4x4::Inverse(viewMatrix * GetProjectionMatrix());
        const auto& windowSize{ Application::Get().GetWindow().GetSize() };

        Math::Vec4 BL = Math::Vec4{ -1.0f, -1.0f, 1.0f, 1.0f } * InvViewProjection;
        BL /= BL.w;

        Math::Vec4 BR = Math::Vec4{  1.0f, -1.0f, 1.0f, 1.0f } * InvViewProjection;
        BR /= BR.w;

        Math::Vec4 TL = Math::Vec4{ -1.0f,  1.0f, 1.0f, 1.0f } * InvViewProjection;
        TL /= TL.w;

        const Math::Vec4 Up = TL - BL;
        const Math::Vec4 Right = BR - BL;

        const Math::Vec4 P = BL + Right * (cursorPos.x / windowSize.x) + Up * (1.0f - cursorPos.y / windowSize.y);

        return Math::Vec3{ P.x, P.y, P.z };
    }
}
