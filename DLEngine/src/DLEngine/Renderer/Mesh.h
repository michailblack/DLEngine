#pragma once
#include "DLEngine/Math/Mat4x4.h"
#include "DLEngine/Math/Primitives.h"

#include "DLEngine/Renderer/TriangleOctree.h"

class Mesh
{
public:
    Mesh(const std::vector<Math::Triangle>& triangles);

    static const Mesh& GetUnitCube();

    const std::vector<Math::Triangle>& GetTriangles() const { return m_Triangles; }

private:
    std::vector<Math::Triangle> m_Triangles;
    TriangleOctree m_Octree;
};
