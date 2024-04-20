#pragma once
#include "DLEngine/Math/Primitives.h"

#include "DLEngine/Renderer/TriangleOctree.h"

class Mesh
{
public:
    Mesh(const std::vector<Math::Triangle>& triangles);

    const std::vector<Math::Triangle>& GetTriangles() const { return m_Triangles; }
    const TriangleOctree& GetOctree() const { return m_Octree; }

    static const Mesh& GetUnitCube();

private:
    std::vector<Math::Triangle> m_Triangles;
    TriangleOctree m_Octree;
};
