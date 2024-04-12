#pragma once
#include "DLEngine/Math/Mat4x4.h"
#include "DLEngine/Math/Vec3.h"

class Camera
{
public:
    Camera(float fov, float aspectRatio, float zNear, float zFar);

    void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; }

    void Translate(const Math::Vec3& translation) { m_Position += translation; }

    void RotateForward(float angle);
    void RotateRight(float angle);
    void RotateUp(float angle);

    Math::Vec3 GetPosition() const { return m_Position; }

    Math::Vec3 GetRight() const { return m_Right; }
    Math::Vec3 GetUp() const { return m_Up; }
    Math::Vec3 GetForward() const { return m_Forward; }

    Math::Mat4x4 GetProjectionMatrix() const;
    Math::Mat4x4 GetViewMatrix() const;

private:
    Math::Vec3 m_Position { 0.0f, 0.0f, 0.0f };

    Math::Vec3 m_Right   { 1.0f, 0.0f, 0.0f };
    Math::Vec3 m_Up      { 0.0f, 1.0f, 0.0f };
    Math::Vec3 m_Forward { 0.0f, 0.0f, 1.0f };

    float m_FOV;
    float m_AspectRatio;
    float m_Near;
    float m_Far;
};
