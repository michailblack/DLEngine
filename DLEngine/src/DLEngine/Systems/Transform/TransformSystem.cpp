#include "dlpch.h"
#include "TransformSystem.h"

#include "DLEngine/Core/solid_vector.h"

#include "DLEngine/DirectX/RenderCommand.h"
#include "DLEngine/DirectX/StructuredBuffer.h"

namespace DLEngine
{
    namespace
    {
        struct
        {
            solid_vector<Math::Mat4x4> Transforms;
            solid_vector<Math::Mat4x4> InvTransforms;

            RStructuredBuffer TransformSB;
            RStructuredBuffer InvTransformSB;

            bool ShouldUpdate{ false };
        } s_Data;
    }

    void TransformSystem::Init()
    {
        StructuredBufferDesc desc{};
        desc.StructureSize = sizeof(Math::Mat4x4);

        StructuredBuffer sb{};
        sb.Create(desc);
        s_Data.TransformSB.Create(sb);
        s_Data.InvTransformSB.Create(sb);

        DL_LOG_INFO("Transform System Initialized");
    }

    uint32_t TransformSystem::AddTransform(const Math::Mat4x4& transform) noexcept
    {
        const uint32_t id{ s_Data.Transforms.insert(transform) };
        s_Data.InvTransforms.insert(Math::Mat4x4::Inverse(transform));

        s_Data.ShouldUpdate = true;

        return id;
    }

    uint32_t TransformSystem::AddTransform(const Math::Mat4x4& transform, const Math::Mat4x4& invTransform) noexcept
    {
        const uint32_t id{ s_Data.Transforms.insert(transform) };
        s_Data.InvTransforms.insert(invTransform);

        s_Data.ShouldUpdate = true;

        return id;
    }

    void TransformSystem::ReplaceTransform(uint32_t id, const Math::Mat4x4& transform) noexcept
    {
        s_Data.ShouldUpdate = true;
        s_Data.Transforms[id] = transform;
        s_Data.InvTransforms[id] = Math::Mat4x4::Inverse(transform);
    }

    uint32_t TransformSystem::GetArrayIndex(uint32_t id) noexcept
    {
        return s_Data.Transforms.getIndex(id);
    }

    void TransformSystem::Update()
    {
        if (s_Data.ShouldUpdate)
        {
            s_Data.ShouldUpdate = false;

            if (s_Data.Transforms.capacity() != s_Data.TransformSB.GetStructuredBuffer().GetElementsCount())
            {
                s_Data.TransformSB.Resize(s_Data.Transforms.capacity());
                s_Data.InvTransformSB.Resize(s_Data.InvTransforms.capacity());
            }

            const auto* transformsData{ s_Data.Transforms.data() };
            const auto* invTransformsData{ s_Data.InvTransforms.data() };
            auto* transforms{ static_cast<Math::Mat4x4*>(s_Data.TransformSB.GetStructuredBuffer().Map()) };
            auto* invTransforms{ static_cast<Math::Mat4x4*>(s_Data.InvTransformSB.GetStructuredBuffer().Map()) };

            const uint32_t transformsCount { s_Data.Transforms.size() };
            for (uint32_t i{ 0u }; i < transformsCount; ++i)
            {
               transforms[i] = Math::Mat4x4::Transpose(transformsData[i]);
               invTransforms[i] = Math::Mat4x4::Transpose(invTransformsData[i]);
            }
            s_Data.TransformSB.GetStructuredBuffer().Unmap();
            s_Data.InvTransformSB.GetStructuredBuffer().Unmap();
        }

        RenderCommand::SetShaderResources(0u, ShaderStage::All, { s_Data.TransformSB.GetSRV(), s_Data.InvTransformSB.GetSRV() });
    }

    Math::Mat4x4& TransformSystem::GetTransform(uint32_t id) noexcept
    {
        s_Data.ShouldUpdate = true;
        return s_Data.Transforms[id];
    }

    Math::Mat4x4& TransformSystem::GetInvTransform(uint32_t id) noexcept
    {
        s_Data.ShouldUpdate = true;
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
        s_Data.ShouldUpdate = true;
        s_Data.Transforms[id] = s_Data.Transforms[id] * Math::Mat4x4::Translate(translation);
        s_Data.InvTransforms[id] = Math::Mat4x4::Inverse(s_Data.Transforms[id]);
    }
}
