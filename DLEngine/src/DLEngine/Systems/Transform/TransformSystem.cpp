#include "dlpch.h"
#include "TransformSystem.h"

namespace DLEngine
{
    namespace
    {
        struct
        {
            std::vector<Math::Mat4x4> Transforms;
            std::vector<Math::Mat4x4> InvTransforms;
        } s_TransformData;
    }

    void TransformSystem::Init()
    {
        DL_LOG_INFO("Transform System Initialized");
    }

    uint32_t TransformSystem::AddTransform(const Math::Mat4x4& transform) noexcept
    {
        s_TransformData.Transforms.push_back(transform);
        s_TransformData.InvTransforms.push_back(Math::Mat4x4::Inverse(transform));

        return static_cast<uint32_t>(s_TransformData.Transforms.size()) - 1u;
    }

    uint32_t TransformSystem::AddTransform(const Math::Mat4x4& transform, const Math::Mat4x4& invTransform) noexcept
    {
        s_TransformData.Transforms.push_back(transform);
        s_TransformData.InvTransforms.push_back(invTransform);

        return static_cast<uint32_t>(s_TransformData.Transforms.size()) - 1u;
    }

    Math::Mat4x4& TransformSystem::GetTransform(uint32_t id) noexcept
    {
        DL_ASSERT(id < static_cast<uint32_t>(s_TransformData.Transforms.size()), "Transform ID out of range");

        return s_TransformData.Transforms[id];
    }

    Math::Mat4x4& TransformSystem::GetInvTransform(uint32_t id) noexcept
    {
        DL_ASSERT(id < static_cast<uint32_t>(s_TransformData.InvTransforms.size()), "Transform ID out of range");

        return s_TransformData.InvTransforms[id];
    }

    Math::Vec3 TransformSystem::TransformPoint(const Math::Vec3& point, const Math::Mat4x4& transform) noexcept
    {
        return Math::Vec4{
            Math::Vec4{ point, 1.0f } * transform
        }.xyz();
    }

    Math::Vec3 TransformSystem::TransformDirection(const Math::Vec3& direction, const Math::Mat4x4& transform) noexcept
    {
        return Math::Vec4{
            Math::Vec4{ direction, 0.0f } *transform
        }.xyz();
    }

    void TransformSystem::Translate(uint32_t id, const Math::Vec3& translation) noexcept
    {
        DL_ASSERT(id < static_cast<uint32_t>(s_TransformData.Transforms.size()), "Transform ID out of range");

        s_TransformData.Transforms[id] = s_TransformData.Transforms[id] * Math::Mat4x4::Translate(translation);
        s_TransformData.InvTransforms[id] = Math::Mat4x4::Inverse(s_TransformData.Transforms[id]);
    }
}
