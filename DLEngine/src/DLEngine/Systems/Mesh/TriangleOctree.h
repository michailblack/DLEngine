#pragma once
#include "DLEngine/Math/Primitives.h"

namespace DLEngine
{
    class Mesh;

    class TriangleOctree
    {
    private:
        struct OctreeNode
        {
            Math::AABB BoundingBox;
            uint32_t FirstChild{ 0u };
            uint32_t FirstTriangle{ 0u };
            uint32_t TriangleCount{ 0u };
        };

    public:
        TriangleOctree(const Mesh& targetMesh) noexcept;

        void Build();

        // Ray must be in mesh space
        bool Intersects(const Math::Ray& ray, uint32_t& outTriangleIndex) const noexcept;

    private:
        void Subdivide(uint32_t nodeIndex);
        void ShrinkNodes();

        bool NodeHasChildren(uint32_t nodeIndex) const;

        static uint32_t GetNodeDepth(uint32_t nodeIndex) noexcept;
        static uint32_t CountMaxElementsWithDepth(uint32_t depth) noexcept;

    private:
        std::vector<OctreeNode> m_Nodes;
        std::vector<uint32_t> m_TriangleIndices;

        uint32_t m_EmptyLeafIndicator{ 0u };

        const uint32_t m_MaxTrianglesPerNode{ 1u };
        uint32_t m_MaxDepth{ 1u };

        const Mesh& m_TargetMesh;
    };
}
