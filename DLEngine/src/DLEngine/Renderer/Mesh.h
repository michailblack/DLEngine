#pragma once
#include "DLEngine/Math/Primitives.h"

#include "DLEngine/Renderer/TriangleOctree.h"

namespace DLEngine
{
    class Mesh
    {
    public:
        Mesh(const std::vector<Math::Triangle>& triangles) noexcept;

        const std::vector<Math::Triangle>& GetTriangles() const noexcept { return m_Triangles; }
        const TriangleOctree& GetOctree() const noexcept { return m_Octree; }

        static const Mesh& GetUnitCube() noexcept;

    private:
        std::vector<Math::Triangle> m_Triangles;
        TriangleOctree m_Octree;
    };
}
