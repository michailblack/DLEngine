#pragma once
#include "DLEngine/Math/Mat4x4.h"
#include "DLEngine/Math/Primitives.h"

#include "DLEngine/Renderer/Instance.h"

namespace DLEngine
{
    class IDragger
    {
    public:
        IDragger(const Math::Vec3& startPoint, float distance) noexcept
            : m_StartPoint(startPoint)
            , m_Distance(distance)
        {}
        virtual ~IDragger() = default;

        virtual void Drag(const Math::Ray& endRay) = 0;

    protected:
        Math::Vec3 m_StartPoint;
        float m_Distance;
    };

    class InstanceDragger : public IDragger
    {
    public:
        InstanceDragger(const Math::Vec3& startPoint, float distance, Ref<Instance> instance) noexcept
            : IDragger(startPoint, distance)
            , m_Instance(instance)
        {
            DL_ASSERT(m_Instance, "Instance is nullptr");
            DL_ASSERT(m_Instance->HasUniform("TRANSFORM"), "Instance does not have a TRANSFORM uniform");
        }

        void Drag(const Math::Ray& endRay) override
        {
            const Math::Vec3 endPoint{ endRay.Origin + endRay.Direction * m_Distance };
            const Math::Vec3 translation{ endPoint - m_StartPoint };

            const auto& transform{ m_Instance->Get<Math::Mat4x4>("TRANSFORM") };
            const auto& finalTransform{ transform * Math::Mat4x4::Translate(translation) };
            m_Instance->Set("TRANSFORM", Buffer{ &finalTransform, sizeof(Math::Mat4x4) });

            m_StartPoint = endPoint;
        }

    protected:
        Ref<Instance> m_Instance;
    };
}