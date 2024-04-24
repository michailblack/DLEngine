#include "dlpch.h"
#include "TriangleOctree.h"

#include <complex>
#include <numeric>

TriangleOctree::TriangleOctree(const std::vector<Math::Triangle>& triangles)
    : m_EmptyLeafIndicator(static_cast<uint32_t>(triangles.size()))
    , m_MaxDepth(static_cast<uint32_t>(std::ceil(std::log(static_cast<float>(triangles.size())) / std::log(8))))
{
    m_Nodes.resize(triangles.size() * 8 - 1,
        OctreeNode {
            .BoundingBox = Math::AABB {},
            .FirstChild = m_EmptyLeafIndicator,
            .FirstTriangle = 0,
            .TriangleCount = 0
        });
    m_TriangleIndices.resize(triangles.size());
    std::iota(m_TriangleIndices.begin(), m_TriangleIndices.end(), 0);

    Build(triangles);
}

bool TriangleOctree::NodeHasChildren(uint32_t nodeIndex) const
{
    return m_Nodes[nodeIndex].FirstChild != m_EmptyLeafIndicator;
}

void TriangleOctree::Build(const std::vector<Math::Triangle>& triangles)
{
    constexpr uint32_t rootIndex { 0 };
    ConstructInitialBoundingBox(triangles);
    Subdivide(rootIndex, triangles);
    ShrinkNodes();
}

void TriangleOctree::ConstructInitialBoundingBox(const std::vector<Math::Triangle>& triangles)
{
    constexpr uint32_t rootIndex { 0 };
    OctreeNode& root { m_Nodes[rootIndex] };
    root.FirstChild = 0;
    root.FirstTriangle = 0;
    root.TriangleCount = static_cast<uint32_t>(m_TriangleIndices.size());

    root.BoundingBox = Math::AABB {
        .Min = Math::Vec3 { std::numeric_limits<float>::max() },
        .Max = Math::Vec3 { -std::numeric_limits<float>::max() }
    };

    for (uint32_t i { root.FirstTriangle }; i < root.FirstTriangle + root.TriangleCount; ++i)
    {
        const Math::Triangle& triangle { triangles[m_TriangleIndices[i]] };
        root.BoundingBox.Min = Math::Min(root.BoundingBox.Min, Math::Min(triangle.V0, Math::Min(triangle.V1, triangle.V2)));
        root.BoundingBox.Max = Math::Max(root.BoundingBox.Max, Math::Max(triangle.V0, Math::Max(triangle.V1, triangle.V2)));
    }
}

void TriangleOctree::Subdivide(uint32_t nodeIndex, const std::vector<Math::Triangle>& triangles)
{
    OctreeNode& node { m_Nodes[nodeIndex] };
    if (node.TriangleCount <= m_MaxTrianglesPerNode || GetCurrentDepth(nodeIndex) >= m_MaxDepth)
        return;

    const Math::Vec3 center { (node.BoundingBox.Min + node.BoundingBox.Max) / 2.0f };
    const Math::Vec3 size { node.BoundingBox.Max - node.BoundingBox.Min };

    const Math::Vec3 quarterSize { size / 4.0f };

    const Math::Vec3 centers[]
    {
        center + Math::Vec3 { -quarterSize.x, -quarterSize.y, -quarterSize.z },
        center + Math::Vec3 {  quarterSize.x, -quarterSize.y, -quarterSize.z },
        center + Math::Vec3 { -quarterSize.x,  quarterSize.y, -quarterSize.z },
        center + Math::Vec3 {  quarterSize.x,  quarterSize.y, -quarterSize.z },
        center + Math::Vec3 { -quarterSize.x, -quarterSize.y,  quarterSize.z },
        center + Math::Vec3 {  quarterSize.x, -quarterSize.y,  quarterSize.z },
        center + Math::Vec3 { -quarterSize.x,  quarterSize.y,  quarterSize.z },
        center + Math::Vec3 {  quarterSize.x,  quarterSize.y,  quarterSize.z }
    };

    const uint32_t numTriangleBeforeSubdivision { node.TriangleCount };

    const uint32_t firstChildIndex { nodeIndex * 8 + 1 };
    for (uint32_t i { 0 }; i < 8; ++i)
    {
        OctreeNode& child { m_Nodes[firstChildIndex + i] };
        child.FirstTriangle = node.FirstTriangle;
        child.TriangleCount = 0;
        child.BoundingBox = Math::AABB {
            .Min = centers[i] - quarterSize,
            .Max = centers[i] + quarterSize
        };

        const auto IsPointInsideAABB = [](const Math::Vec3& point, const Math::AABB& aabb) -> bool
        {
            return point.x >= aabb.Min.x && point.x <= aabb.Max.x &&
                   point.y >= aabb.Min.y && point.y <= aabb.Max.y &&
                   point.z >= aabb.Min.z && point.z <= aabb.Max.z;
        };

        const auto IsTriangleInsideAABB = [&IsPointInsideAABB](const Math::Triangle& triangle, const Math::AABB& aabb) -> bool
        {
            return IsPointInsideAABB(triangle.V0, aabb) &&
                   IsPointInsideAABB(triangle.V1, aabb) &&
                   IsPointInsideAABB(triangle.V2, aabb);
        };

        for (uint32_t j { node.FirstTriangle }; j < node.FirstTriangle + node.TriangleCount; ++j)
        {
            const Math::Triangle& triangle { triangles[m_TriangleIndices[j]] };

            if (IsTriangleInsideAABB(triangle, child.BoundingBox))
                std::swap(m_TriangleIndices[j], m_TriangleIndices[child.FirstTriangle + child.TriangleCount++]);
        }

        if (child.TriangleCount > 0)
        {
            node.FirstTriangle += child.TriangleCount;
            node.TriangleCount -= child.TriangleCount;
            Subdivide(firstChildIndex + i, triangles);
        }
        else
            child.FirstChild = m_EmptyLeafIndicator;
    }
    if (node.TriangleCount == numTriangleBeforeSubdivision)
        node.FirstChild = m_EmptyLeafIndicator;
    else
        node.FirstChild = firstChildIndex;
}

void TriangleOctree::ShrinkNodes()
{
    const auto& lastValidLeaf = std::find_if(m_Nodes.rbegin(), m_Nodes.rend(), [](const OctreeNode& node) { return node.TriangleCount > 0; });
    const uint32_t lastValidLeafIndex { static_cast<uint32_t>(std::distance(m_Nodes.begin(), lastValidLeaf.base())) - 1 };

    const uint32_t rightBorder { CountMaxElementsWithDepth(GetCurrentDepth(lastValidLeafIndex)) };
    m_Nodes.resize(rightBorder);

    m_Nodes.shrink_to_fit();
}

uint32_t TriangleOctree::GetCurrentDepth(uint32_t nodeIndex) const
{
    uint32_t depth { 1 };
    while (depth <= m_MaxDepth)
    {
        const uint32_t leftBorder { static_cast<uint32_t>(std::pow(8, depth - 1)) };
        const uint32_t rightBorder { static_cast<uint32_t>(std::pow(8, depth)) };

        if (nodeIndex >= leftBorder && nodeIndex <= rightBorder)
            return depth;

        ++depth;
    }

    return 0;
}

uint32_t TriangleOctree::CountMaxElementsWithDepth(uint32_t depth)
{
    if (depth == 0)
        return 1;

    const uint32_t elementsOnThisLevel { static_cast<uint32_t>(std::pow(8, depth)) };
    return elementsOnThisLevel + CountMaxElementsWithDepth(depth - 1);
}
