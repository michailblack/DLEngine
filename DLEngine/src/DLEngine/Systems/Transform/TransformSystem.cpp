#include "dlpch.h"
#include "TransformSystem.h"

#include "DLEngine/Core/SolidVector.h"
#include "DLEngine/DirectX/ConstantBuffer.h"

namespace DLEngine
{
    namespace
    {
        struct
        {
            SolidVector<Math::Mat4x4> Transforms;
            SolidVector<Math::Mat4x4> InvTransforms;

            ConstantBuffer<Math::Mat4x4> TransformCB;
            ConstantBuffer<Math::Mat4x4> InvTransformCB;
        } s_Data;
    }

    void TransformSystem::Init()
    {
        s_Data.TransformCB.Create(D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT / 4u);
        s_Data.InvTransformCB.Create(D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT / 4u);

        DL_LOG_INFO("Transform System Initialized");
    }

    uint32_t TransformSystem::AddTransform(const Math::Mat4x4& transform) noexcept
    {
        const uint32_t id{ s_Data.Transforms.insert(transform) };
        s_Data.InvTransforms.insert(Math::Mat4x4::Inverse(transform));

        return id;
    }

    uint32_t TransformSystem::AddTransform(const Math::Mat4x4& transform, const Math::Mat4x4& invTransform) noexcept
    {
        const uint32_t id{ s_Data.Transforms.insert(transform) };
        s_Data.InvTransforms.insert(invTransform);

        return id;
    }

    void TransformSystem::ReplaceTransform(uint32_t id, const Math::Mat4x4& transform) noexcept
    {
        s_Data.Transforms[id] = transform;
        s_Data.InvTransforms[id] = Math::Mat4x4::Inverse(transform);
    }

    uint32_t TransformSystem::GetArrayIndex(uint32_t id) noexcept
    {
        return s_Data.Transforms.getIndex(id);
    }

    void TransformSystem::Update()
    {
        s_Data.TransformCB.Set(s_Data.Transforms.data(), s_Data.Transforms.size());
        s_Data.TransformCB.Bind(2u, BIND_ALL);

        s_Data.InvTransformCB.Set(s_Data.InvTransforms.data(), s_Data.InvTransforms.size());
        s_Data.InvTransformCB.Bind(3u, BIND_ALL);
    }

    Math::Mat4x4& TransformSystem::GetTransform(uint32_t id) noexcept
    {
        return s_Data.Transforms[id];
    }

    Math::Mat4x4& TransformSystem::GetInvTransform(uint32_t id) noexcept
    {
        return s_Data.InvTransforms[id];
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
            Math::Vec4{ direction, 0.0f } * transform
        }.xyz();
    }

    void TransformSystem::Translate(uint32_t id, const Math::Vec3& translation) noexcept
    {
        s_Data.Transforms[id] = s_Data.Transforms[id] * Math::Mat4x4::Translate(translation);
        s_Data.InvTransforms[id] = Math::Mat4x4::Inverse(s_Data.Transforms[id]);
    }
}
