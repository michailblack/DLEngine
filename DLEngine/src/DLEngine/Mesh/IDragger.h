#pragma once
#include "DLEngine/Math/Primitives.h"

#include "DLEngine/Mesh/Model.h"

namespace DLEngine
{
    class IDragger
    {
    public:
        IDragger(Math::Vec3 startDraggingPoint, float distanceToDraggingPlane) noexcept
            : m_StartDraggingPoint(startDraggingPoint)
            , m_DistanceToDraggingPlane(distanceToDraggingPlane)
        {}
        virtual ~IDragger() = default;

        virtual void Drag(const Math::Plane& nearPlane, const Math::Ray& endRay) = 0;

    protected:
        Math::Vec3 m_StartDraggingPoint;
        float m_DistanceToDraggingPlane;
    };

    class MeshDragger
        : public IDragger
    {
    public:
        MeshDragger(Math::Vec3 startDraggingPoint, float distanceToDraggingPlane) noexcept
            : IDragger(startDraggingPoint, distanceToDraggingPlane)
        {}
        ~MeshDragger() override = default;
    };
}
