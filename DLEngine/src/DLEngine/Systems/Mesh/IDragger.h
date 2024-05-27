#pragma once
#include "DLEngine/Math/Primitives.h"

namespace DLEngine
{
    class IDragger
    {
    public:
        IDragger(Math::Vec3 startPoint, float distance) noexcept
            : m_StartPoint(startPoint)
            , m_Distance(distance)
        {}
        virtual ~IDragger() = default;

        virtual void Drag(const Math::Ray& endRay) = 0;

    protected:
        Math::Vec3 m_StartPoint;
        float m_Distance;
    };

    class MeshDragger
        : public IDragger
    {
    public:
        MeshDragger(Math::Vec3 startPoint, float distance, uint32_t transformIndex) noexcept
            : IDragger(startPoint, distance)
            , m_TransformIndex(transformIndex)
        {}

        void Drag(const Math::Ray& endRay) override;

    private:
        uint32_t m_TransformIndex;
    };
}
