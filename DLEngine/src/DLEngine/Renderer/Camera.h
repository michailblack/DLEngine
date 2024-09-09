#pragma once
#include "DLEngine/Math/Mat4x4.h"
#include "DLEngine/Math/Vec2.h"
#include "DLEngine/Math/Vec4.h"

namespace DLEngine
{
    class Camera
    {
    public:
        enum class ProjectionType
        {
            None = 0,
            PerspectiveFOV,
            Perspective,
            Orthographic
        };

        struct Frustum
        {
            union
            {
                struct
                {
                    Math::Vec3 NearTopLeft;
                    Math::Vec3 NearTopRight;
                    Math::Vec3 NearBottomLeft;
                    Math::Vec3 NearBottomRight;
                    Math::Vec3 FarTopLeft;
                    Math::Vec3 FarTopRight;
                    Math::Vec3 FarBottomLeft;
                    Math::Vec3 FarBottomRight;
                } Positions;
                std::array<Math::Vec3, 8u> Corners{};
            };
        };

    public:
        Camera() noexcept = default;
        ~Camera() = default;
        
        Camera(const Camera&) = default;
        Camera(Camera&&) = default;
        Camera& operator=(const Camera&) = default;
        Camera& operator=(Camera&&) = default;

        void SetPerspectiveProjectionFov(float fovAngleY, float aspectRatio, float nearZ, float farZ) noexcept;
        void SetPerspectiveProjection(float viewWidth, float viewHeight, float nearZ, float farZ) noexcept;
        void SetOrthographicProjection(float viewWidth, float viewHeight, float nearZ, float farZ) noexcept;

        void SetView(const Math::Vec3& position, const Math::Vec3& forward, const Math::Vec3& up, const Math::Vec3& right) noexcept;

        void Translate(const Math::Vec3& translation) noexcept { m_Position += translation; }

        void RotateForward(float angle) noexcept;
        void RotateRight(float angle) noexcept;
        void RotateUp(float angle) noexcept;
        void RotateAxis(Math::Vec3 normalizedAxis, float angle) noexcept;

        const Math::Vec3& GetPosition() const noexcept { return m_Position; }

        const Math::Vec3& GetRight() const noexcept { return m_Right; }
        const Math::Vec3& GetUp() const noexcept { return m_Up; }
        const Math::Vec3& GetForward() const noexcept { return m_Forward; }

        float GetNearZ() const noexcept { return m_NearZ; }
        float GetFarZ() const noexcept { return m_FarZ; }

        Math::Mat4x4 GetProjectionMatrix() const noexcept;
        Math::Mat4x4 GetViewMatrix() const noexcept;

        Math::Vec3 ConstructFrustumPos(const Math::Vec3& ndc) const noexcept;
        Math::Vec3 ConstructFrustumPosNoTranslation(const Math::Vec3& ndc) const noexcept;

        Frustum ConstructFrustum() const noexcept;

    private:
        Math::Vec3 m_Position{ 0.0f, 0.0f, 0.0f };

        Math::Vec3 m_Right{ 1.0f, 0.0f, 0.0f };
        Math::Vec3 m_Up{ 0.0f, 1.0f, 0.0f };
        Math::Vec3 m_Forward{ 0.0f, 0.0f, 1.0f };

        union
        {
            float m_FovAngleY;
            float m_ViewWidth{ 0.0f };
        };

        union
        {
            float m_AspectRatio;
            float m_ViewHeight{ 0.0f };
        };

        float m_NearZ{ 100.0f };
        float m_FarZ{ 0.001f };

        ProjectionType m_ProjectionType{ ProjectionType::None };
    };
}
