#include "dlpch.h"
#include "IDragger.h"

#include "DLEngine/Math/Intersections.h"

void ISphereDragger::Drag(const Math::Plane& nearPlane, const Math::Ray& endRay)
{
    const Math::Plane draggingPlane {
        .Origin = nearPlane.Origin + nearPlane.Normal * m_DistanceToDraggingPlane,
        .Normal = nearPlane.Normal
    };

    Math::IntersectInfo newIntersectInfo;
    Math::Intersects(endRay, draggingPlane, newIntersectInfo);

    m_TargetSphere->Sphere.Center += newIntersectInfo.IntersectionPoint - m_StartDraggingPoint;

    m_StartDraggingPoint = newIntersectInfo.IntersectionPoint;
}

void IPlaneDragger::Drag(const Math::Plane& nearPlane, const Math::Ray& endRay)
{
    const Math::Plane draggingPlane {
        .Origin = nearPlane.Origin + nearPlane.Normal * m_DistanceToDraggingPlane,
        .Normal = nearPlane.Normal
    };

    Math::IntersectInfo newIntersectInfo;
    Math::Intersects(endRay, draggingPlane, newIntersectInfo);

    m_TargetPlane->Plane.Origin += newIntersectInfo.IntersectionPoint - m_StartDraggingPoint;

    m_StartDraggingPoint = newIntersectInfo.IntersectionPoint;
}

void IMeshDragger::Drag(const Math::Plane& nearPlane, const Math::Ray& endRay)
{
    const Math::Plane draggingPlane {
           .Origin = nearPlane.Origin + nearPlane.Normal * m_DistanceToDraggingPlane,
           .Normal = nearPlane.Normal
       };

    Math::IntersectInfo newIntersectInfo;
    Math::Intersects(endRay, draggingPlane, newIntersectInfo);

    const Math::Vec3 translation = newIntersectInfo.IntersectionPoint - m_StartDraggingPoint;

    m_TargetMesh->Transform = m_TargetMesh->Transform * Math::Mat4x4::Translate(translation);
    m_TargetMesh->InvTransform = Math::Mat4x4::Inverse(m_TargetMesh->Transform);

    m_StartDraggingPoint = newIntersectInfo.IntersectionPoint;
}
