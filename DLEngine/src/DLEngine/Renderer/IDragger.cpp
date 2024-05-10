#include "dlpch.h"
#include "IDragger.h"

#include "DLEngine/Math/Intersections.h"

#include "DLEngine/Renderer/Model.h"

namespace DLEngine
{
    void SphereDragger::Drag(const Math::Plane& nearPlane, const Math::Ray& endRay)
    {
        const Math::Plane draggingPlane{
            .Origin = nearPlane.Origin + nearPlane.Normal * m_DistanceToDraggingPlane,
            .Normal = nearPlane.Normal
        };

        Math::IntersectInfo newIntersectInfo;
        Math::Intersects(endRay, draggingPlane, newIntersectInfo);

        m_TargetSphere->Sphere.Center += newIntersectInfo.IntersectionPoint - m_StartDraggingPoint;

        m_StartDraggingPoint = newIntersectInfo.IntersectionPoint;
    }

    void PlaneDragger::Drag(const Math::Plane& nearPlane, const Math::Ray& endRay)
    {
        const Math::Plane draggingPlane{
            .Origin = nearPlane.Origin + nearPlane.Normal * m_DistanceToDraggingPlane,
            .Normal = nearPlane.Normal
        };

        Math::IntersectInfo newIntersectInfo;
        Math::Intersects(endRay, draggingPlane, newIntersectInfo);

        m_TargetPlane->Plane.Origin += newIntersectInfo.IntersectionPoint - m_StartDraggingPoint;

        m_StartDraggingPoint = newIntersectInfo.IntersectionPoint;
    }

    void MeshDragger::Drag(const Math::Plane& nearPlane, const Math::Ray& endRay)
    {
        const Math::Plane draggingPlane{
            .Origin = nearPlane.Origin + nearPlane.Normal * m_DistanceToDraggingPlane,
            .Normal = nearPlane.Normal
        };

        Math::IntersectInfo newIntersectInfo;
        Math::Intersects(endRay, draggingPlane, newIntersectInfo);

        const Math::Vec3 translation = newIntersectInfo.IntersectionPoint - m_StartDraggingPoint;

        auto& srcMesh{ m_TargetMesh->Model->GetMesh(m_TargetMesh->MeshIndex) };
        auto& srcMeshInstance{ srcMesh.GetInstance(m_TargetMesh->MeshInstanceIndex) };
        auto& srcMeshInvInstance{ srcMesh.GetInvInstance(m_TargetMesh->MeshInstanceIndex) };

        srcMeshInstance = srcMeshInstance * Math::Mat4x4::Translate(translation);
        srcMeshInvInstance = Math::Mat4x4::Inverse(srcMeshInstance);

        m_StartDraggingPoint = newIntersectInfo.IntersectionPoint;
    }
}
