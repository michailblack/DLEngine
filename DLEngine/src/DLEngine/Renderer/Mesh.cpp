#include "dlpch.h"
#include "Mesh.h"

namespace DLEngine
{
    Mesh::Mesh(const std::vector<Math::Triangle>& triangles) noexcept
        : m_Triangles(triangles)
        , m_Octree(triangles)
    {
    }

    const Mesh& Mesh::GetUnitCube() noexcept
    {
        static const Mesh s_UnitCube{
            {
                // Front
                { Math::Vec3 { -0.5f, -0.5f, -0.5f }, Math::Vec3 { 0.5f, -0.5f, -0.5f }, Math::Vec3 { 0.5f, 0.5f, -0.5f } },
                { Math::Vec3 { 0.5f, 0.5f, -0.5f }, Math::Vec3 { -0.5f, 0.5f, -0.5f }, Math::Vec3 { -0.5f, -0.5f, -0.5f } },

                // Back
                { Math::Vec3 { 0.5f, -0.5f, 0.5f }, Math::Vec3 { -0.5f, -0.5f, 0.5f }, Math::Vec3 { 0.5f, 0.5f, 0.5f } },
                { Math::Vec3 { 0.5f, 0.5f, 0.5f }, Math::Vec3 { -0.5f, -0.5f, 0.5f }, Math::Vec3 { -0.5f, 0.5f, 0.5f } },

                // Left
                { Math::Vec3 { -0.5f, -0.5f, 0.5f }, Math::Vec3 { -0.5f, -0.5f, -0.5f }, Math::Vec3 { -0.5f, 0.5f, -0.5f } },
                { Math::Vec3 { -0.5f, 0.5f, -0.5f }, Math::Vec3 { -0.5f, 0.5f, 0.5f }, Math::Vec3 { -0.5f, -0.5f, 0.5f } },

                // Right
                { Math::Vec3 { 0.5f, -0.5f, -0.5f }, Math::Vec3 { 0.5f, -0.5f, 0.5f }, Math::Vec3 { 0.5f, 0.5f, 0.5f } },
                { Math::Vec3 { 0.5f, 0.5f, 0.5f }, Math::Vec3 { 0.5f, 0.5f, -0.5f }, Math::Vec3 { 0.5f, -0.5f, -0.5f } },

                // Top
                { Math::Vec3 { -0.5f, 0.5f, -0.5f }, Math::Vec3 { 0.5f, 0.5f, -0.5f }, Math::Vec3 { 0.5f, 0.5f, 0.5f } },
                { Math::Vec3 { 0.5f, 0.5f, 0.5f }, Math::Vec3 { -0.5f, 0.5f, 0.5f }, Math::Vec3 { -0.5f, 0.5f, -0.5f } },

                // Bottom
                { Math::Vec3 { -0.5f, -0.5f, -0.5f }, Math::Vec3 { -0.5f, -0.5f, 0.5f }, Math::Vec3 { 0.5f, -0.5f, 0.5f } },
                { Math::Vec3 { 0.5f, -0.5f, 0.5f }, Math::Vec3 { 0.5f, -0.5f, -0.5f }, Math::Vec3 { -0.5f, -0.5f, -0.5f } },
            },
        };

        return s_UnitCube;
    }
}
