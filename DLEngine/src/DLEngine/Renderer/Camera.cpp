#include "dlpch.h"
#include "Camera.h"

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

Math::Mat4x4 Camera::GetProjectionMatrix() const noexcept
{
    return Math::Mat4x4::Perspective(m_FOV, m_AspectRatio, m_Near, m_Far);
}

Math::Mat4x4 Camera::GetViewMatrix() const noexcept
{
    return Math::Mat4x4::View(m_Right, m_Up, m_Forward, m_Position);
}
