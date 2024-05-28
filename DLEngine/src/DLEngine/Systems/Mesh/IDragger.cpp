#include "dlpch.h"
#include "IDragger.h"

#include "DLEngine/Systems/Transform/TransformSystem.h"

namespace DLEngine
{
    void MeshDragger::Drag(const Math::Ray& endRay)
    {
        const Math::Vec3 endPoint{ endRay.Origin + endRay.Direction * m_Distance };
        TransformSystem::Translate(m_TransformIndex, endPoint - m_StartPoint);

        m_StartPoint = endPoint;
    }
}
