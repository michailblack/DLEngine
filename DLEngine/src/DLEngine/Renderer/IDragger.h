#pragma once
#include "DLEngine/Math/Primitives.h"

#include "DLEngine/Renderer/Entity.h"

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

    class SphereDragger
        : public IDragger
    {
    public:
        SphereDragger(const Ref<SphereInstance>& targetSphere, Math::Vec3 startDraggingPoint, float distanceToDraggingPlane) noexcept
            : IDragger(startDraggingPoint, distanceToDraggingPlane)
            , m_TargetSphere(targetSphere)
        {}
        ~SphereDragger() override = default;

        void Drag(const Math::Plane& nearPlane, const Math::Ray& endRay) override;

    private:
        Ref<SphereInstance> m_TargetSphere;
    };

    class PlaneDragger
        : public IDragger
    {
    public:
        PlaneDragger(const Ref<PlaneInstance>& targetPlane, Math::Vec3 startDraggingPoint, float distanceToDraggingPlane) noexcept
            : IDragger(startDraggingPoint, distanceToDraggingPlane)
            , m_TargetPlane(targetPlane)
        {}
        ~PlaneDragger() override = default;

        void Drag(const Math::Plane& nearPlane, const Math::Ray& endRay) override;

    private:
        Ref<PlaneInstance> m_TargetPlane;
    };

    class MeshDragger
        : public IDragger
    {
    public:
        MeshDragger(const Ref<MeshInstance>& targetMesh, Math::Vec3 startDraggingPoint, float distanceToDraggingPlane) noexcept
            : IDragger(startDraggingPoint, distanceToDraggingPlane)
            , m_TargetMesh(targetMesh)
        {}
        ~MeshDragger() override = default;

        void Drag(const Math::Plane& nearPlane, const Math::Ray& endRay) override;

    private:
        Ref<MeshInstance> m_TargetMesh;
    };
}
