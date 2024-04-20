﻿#pragma once
#include "DLEngine/Math/Primitives.h"

class TriangleOctree
{
private:
    struct OctreeNode
    {
        Math::AABB BoundingBox;
        uint32_t FirstChild { 0 };
        uint32_t FirstTriangle { 0 };
        uint32_t TriangleCount { 0 };
    };

public:
    TriangleOctree(const std::vector<Math::Triangle>& triangles);

    const std::vector<OctreeNode>& GetNodes() const { return m_Nodes; }

    bool NodeHasChildren(uint32_t nodeIndex) const;

private:
    void Build(const std::vector<Math::Triangle>& triangles);
    void ConstructInitialBoundingBox(const std::vector<Math::Triangle>& triangles);
    void Subdivide(uint32_t nodeIndex, const std::vector<Math::Triangle>& triangles);
    void ShrinkNodes();

    uint32_t GetCurrentDepth(uint32_t nodeIndex) const;
    static uint32_t CountMaxElementsWithDepth(uint32_t depth);

private:
    std::vector<OctreeNode> m_Nodes;
    //const std::vector<Math::Triangle>* m_Triangles;
    std::vector<uint32_t> m_TriangleIndices;

    const uint32_t m_EmptyLeafIndicator;

    const uint32_t m_MaxTrianglesPerNode { 1 };
    const uint32_t m_MaxDepth;
};
