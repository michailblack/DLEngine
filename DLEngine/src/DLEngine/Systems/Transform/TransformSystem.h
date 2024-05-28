#pragma once
#include "DLEngine/Math/Mat4x4.h"
#include "DLEngine/Math/Vec4.h"
#include "DLEngine/Math/Vec3.h"

namespace DLEngine
{
    class TransformSystem
    {
    public:
        static void Init();

        static uint32_t AddTransform(const Math::Mat4x4& transform) noexcept;
        static uint32_t AddTransform(const Math::Mat4x4& transform, const Math::Mat4x4& invTransform) noexcept;

        static Math::Mat4x4& GetTransform(uint32_t id) noexcept;
        static Math::Mat4x4& GetInvTransform(uint32_t id) noexcept;

        static Math::Vec3 TransformPoint(const Math::Vec3& point, const Math::Mat4x4& transform) noexcept;
        static Math::Vec3 TransformDirection(const Math::Vec3& direction, const Math::Mat4x4& transform) noexcept;

        static void Translate(uint32_t id, const Math::Vec3& translation) noexcept;
    };
}
