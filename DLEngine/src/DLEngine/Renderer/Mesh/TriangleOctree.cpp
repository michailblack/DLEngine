#include "dlpch.h"
#include "TriangleOctree.h"

#include "DLEngine/Math/Intersections.h"

#include "DLEngine/Renderer/Mesh/Mesh.h"

#include <numeric>

namespace DLEngine
{
    void TriangleOctree::Rebuild(const Submesh& targetSubmesh) noexcept
    {
        m_Nodes.clear();
        m_TriangleIndices.clear();

        const std::vector<Submesh::Triangle>& triangles{ targetSubmesh.GetTriangles() };
        m_EmptyLeafIndicator = static_cast<uint32_t>(triangles.size());
        m_MaxDepth = static_cast<uint32_t>(std::ceil(std::log(static_cast<float>(triangles.size())) / std::log(8)));

        m_Nodes.resize(CountMaxElementsWithDepth(m_MaxDepth),
            OctreeNode{
                .BoundingBox = Math::AABB{},
                .FirstChild = m_EmptyLeafIndicator,
                .FirstTriangle = 0u,
                .TriangleCount = 0u
            });
        m_TriangleIndices.resize(triangles.size());
        std::iota(m_TriangleIndices.begin(), m_TriangleIndices.end(), 0u);

        constexpr uint32_t rootIndex{ 0u };
        OctreeNode& root{ m_Nodes[rootIndex] };

        constexpr Math::Vec3 eps{ 1e-5f };
        root.BoundingBox = targetSubmesh.GetBoundingBox();
        root.BoundingBox.Min -= eps;
        root.BoundingBox.Max += eps;

        root.FirstChild = 0u;
        root.FirstTriangle = 0u;
        root.TriangleCount = static_cast<uint32_t>(m_TriangleIndices.size());

        Subdivide(targetSubmesh, rootIndex);

        ShrinkNodes();
    }

    bool TriangleOctree::NodeHasChildren(uint32_t nodeIndex) const noexcept
    {
        return m_Nodes[nodeIndex].FirstChild != m_EmptyLeafIndicator;
    }

    bool TriangleOctree::Intersects(const Submesh& targetSubmesh, const Math::Ray& ray, uint32_t& outTriangleIndex) const noexcept
    {
        bool intersects{ false };

        constexpr uint32_t rootIndex{ 0u };
        std::stack<uint32_t> stack;
        stack.push(rootIndex);

        const auto& triangles{ targetSubmesh.GetTriangles() };
        const auto& vertices{ targetSubmesh.GetVertices() };

        Math::IntersectInfo intersectInfo{};

        while (!stack.empty())
        {
            const uint32_t nodeIndex{ stack.top() };
            stack.pop();

            const OctreeNode& node{ m_Nodes[nodeIndex] };

            if (!Math::Intersects(ray, node.BoundingBox))
                continue;

            if (node.TriangleCount > 0u)
            {
                for (uint32_t i{ node.FirstTriangle }; i < node.FirstTriangle + node.TriangleCount; ++i)
                {
                    const auto& triangle{ triangles[m_TriangleIndices[i]] };
                    Math::Triangle triangleToCheck{
                        .V0 = vertices[triangle.Indices[0]].Position,
                        .V1 = vertices[triangle.Indices[1]].Position,
                        .V2 = vertices[triangle.Indices[2]].Position
                    };

                    if (Math::Intersects(ray, triangleToCheck, intersectInfo))
                    {
                        outTriangleIndex = m_TriangleIndices[i];
                        intersects = true;
                    }
                }
            }

            if (NodeHasChildren(nodeIndex))
            {
                for (uint32_t i{ 0u }; i < 8u; ++i)
                    stack.push(node.FirstChild + i);
            }
        }

        return intersects;
    }

    void TriangleOctree::Subdivide(const Submesh& targetSubmesh, uint32_t nodeIndex)
    {
        const std::vector<Submesh::Triangle>& triangles{ targetSubmesh.GetTriangles() };

        OctreeNode& node{ m_Nodes[nodeIndex] };
        if (node.TriangleCount <= m_MaxTrianglesPerNode || GetNodeDepth(nodeIndex) >= m_MaxDepth)
            return;

        const Math::Vec3 center{ (node.BoundingBox.Min + node.BoundingBox.Max) / 2.0f };
        const Math::Vec3 size{ node.BoundingBox.Max - node.BoundingBox.Min };

        const Math::Vec3 quarterSize{ size / 4.0f };

        const Math::Vec3 centers[]
        {
            center + Math::Vec3{ -quarterSize.x, -quarterSize.y, -quarterSize.z },
            center + Math::Vec3{  quarterSize.x, -quarterSize.y, -quarterSize.z },
            center + Math::Vec3{ -quarterSize.x,  quarterSize.y, -quarterSize.z },
            center + Math::Vec3{  quarterSize.x,  quarterSize.y, -quarterSize.z },
            center + Math::Vec3{ -quarterSize.x, -quarterSize.y,  quarterSize.z },
            center + Math::Vec3{  quarterSize.x, -quarterSize.y,  quarterSize.z },
            center + Math::Vec3{ -quarterSize.x,  quarterSize.y,  quarterSize.z },
            center + Math::Vec3{  quarterSize.x,  quarterSize.y,  quarterSize.z }
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

        const std::vector<Submesh::Vertex>& vertices{ targetSubmesh.GetVertices() };

        const uint32_t numTriangleBeforeSubdivision{ node.TriangleCount };

        const uint32_t firstChildIndex{ nodeIndex * 8 + 1 };
        for (uint32_t i{ 0 }; i < 8; ++i)
        {
            OctreeNode& child{ m_Nodes[firstChildIndex + i] };
            child.FirstTriangle = node.FirstTriangle;
            child.TriangleCount = 0;

            constexpr Math::Vec3 eps{ 1e-5f };
            child.BoundingBox = Math::AABB{
                .Min = centers[i] - quarterSize - eps,
                .Max = centers[i] + quarterSize + eps
            };

            for (uint32_t j{ node.FirstTriangle }; j < node.FirstTriangle + node.TriangleCount; ++j)
            {
                const Submesh::Triangle& triangle{ triangles[m_TriangleIndices[j]] };

                Math::Triangle triangleToCheck{
                    .V0 = vertices[triangle.Indices[0]].Position,
                    .V1 = vertices[triangle.Indices[1]].Position,
                    .V2 = vertices[triangle.Indices[2]].Position
                };

                if (IsTriangleInsideAABB(triangleToCheck, child.BoundingBox))
                    std::swap(m_TriangleIndices[j], m_TriangleIndices[child.FirstTriangle + child.TriangleCount++]);
            }

            if (child.TriangleCount > 0)
            {
                node.FirstTriangle += child.TriangleCount;
                node.TriangleCount -= child.TriangleCount;
                Subdivide(targetSubmesh, firstChildIndex + i);
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
        const uint32_t lastValidLeafIndex{ static_cast<uint32_t>(std::distance(m_Nodes.begin(), lastValidLeaf.base())) - 1 };

        const uint32_t rightBorder{ CountMaxElementsWithDepth(GetNodeDepth(lastValidLeafIndex)) };
        m_Nodes.resize(rightBorder);

        m_Nodes.shrink_to_fit();
    }

    uint32_t TriangleOctree::GetNodeDepth(uint32_t nodeIndex) noexcept
    {
        if (nodeIndex == 0)
            return 0;

        uint32_t depth{ 1u };
        uint32_t offset{ 1u };
        while (true)
        {
            uint32_t nodesOnThisLevel{ static_cast<uint32_t>(std::pow(8, depth)) };
            if (nodeIndex < offset + nodesOnThisLevel)
                return depth;

            offset += nodesOnThisLevel;
            ++depth;
        }
    }

    uint32_t TriangleOctree::CountMaxElementsWithDepth(uint32_t depth) noexcept
    {
        if (depth == 0)
            return 1;

        const uint32_t elementsOnThisLevel{ static_cast<uint32_t>(std::pow(8, depth)) };
        return elementsOnThisLevel + CountMaxElementsWithDepth(depth - 1);
    }

}