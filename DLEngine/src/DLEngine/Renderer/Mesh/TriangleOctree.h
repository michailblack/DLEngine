#pragma once
#include "DLEngine/Math/Primitives.h"

namespace DLEngine
{
    class Submesh;

    class TriangleOctree
    {
    public:
        struct OctreeNode
        {
            Math::AABB BoundingBox;
            uint32_t FirstChild{ 0u };
            uint32_t FirstTriangle{ 0u };
            uint32_t TriangleCount{ 0u };
        };

    public:
        void Rebuild(const Submesh& targetSubmesh) noexcept;

        const std::vector<OctreeNode>& GetNodes() const noexcept { return m_Nodes; }
        const std::vector<uint32_t>& GetTriangleIndices() const noexcept { return m_TriangleIndices; }

        bool NodeHasChildren(uint32_t nodeIndex) const noexcept;
        bool Intersects(const Submesh& targetSubmesh, const Math::Ray& ray, uint32_t& outTriangleIndex) const noexcept;

    private:
        void Subdivide(const Submesh& targetSubmesh, uint32_t nodeIndex);
        void ShrinkNodes();

        static uint32_t GetNodeDepth(uint32_t nodeIndex) noexcept;
        static uint32_t CountMaxElementsWithDepth(uint32_t depth) noexcept;

    private:
        std::vector<OctreeNode> m_Nodes;
        std::vector<uint32_t> m_TriangleIndices;

        uint32_t m_EmptyLeafIndicator{ 0u };

        uint32_t m_MaxTrianglesPerNode{ 1u };
        uint32_t m_MaxDepth{ 1u };
    };
}