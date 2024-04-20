#pragma once
#include "DLEngine/Math/Primitives.h"

#include "DLEngine/Renderer/Entity.h"

class IDragger
{
public:
    IDragger(Math::Vec3 startDraggingPoint, float distanceToDraggingPlane)
        : m_StartDraggingPoint(startDraggingPoint)
        , m_DistanceToDraggingPlane(distanceToDraggingPlane)
    {}

    virtual void Drag(const Math::Plane& nearPlane, const Math::Ray& endRay) = 0;

protected:
    Math::Vec3 m_StartDraggingPoint;
    float m_DistanceToDraggingPlane;
};

class ISphereDragger
    : public IDragger
{
public:
    ISphereDragger(const Ref<SphereInstance>& targetSphere, Math::Vec3 startDraggingPoint, float distanceToDraggingPlane)
        : IDragger(startDraggingPoint, distanceToDraggingPlane)
        , m_TargetSphere(targetSphere)
    {}

    void Drag(const Math::Plane& nearPlane, const Math::Ray& endRay) override;

private:
    Ref<SphereInstance> m_TargetSphere;
};

class IPlaneDragger
    : public IDragger
{
public:
    IPlaneDragger(const Ref<PlaneInstance>& targetPlane, Math::Vec3 startDraggingPoint, float distanceToDraggingPlane)
        : IDragger(startDraggingPoint, distanceToDraggingPlane)
        , m_TargetPlane(targetPlane)
    {}

    void Drag(const Math::Plane& nearPlane, const Math::Ray& endRay) override;

private:
    Ref<PlaneInstance> m_TargetPlane;
};

class IMeshDragger
    : public IDragger
{
public:
    IMeshDragger(const Ref<MeshInstance>& targetMesh, Math::Vec3 startDraggingPoint, float distanceToDraggingPlane)
        : IDragger(startDraggingPoint, distanceToDraggingPlane)
        , m_TargetMesh(targetMesh)
    {}

    void Drag(const Math::Plane& nearPlane, const Math::Ray& endRay) override;

private:
    Ref<MeshInstance> m_TargetMesh;
};
