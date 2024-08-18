#pragma once
#include "DLEngine/Math/Primitives.h"

#include "DLEngine/Renderer/Mesh/Mesh.h"
#include "DLEngine/Renderer/Mesh/MeshRegistry.h"

namespace DLEngine::Math
{
    bool Intersects(const Ray& ray, const Sphere& sphere, IntersectInfo& outIntersectInfo);
    bool Intersects(const Ray& ray, const Plane& plane, IntersectInfo& outIntersectInfo);
    bool Intersects(const Ray& ray, const Triangle& triangle, IntersectInfo& outIntersectInfo);
    bool Intersects(const Ray& ray, const AABB& aabb);
    bool Intersects(const Ray& ray, const TriangleOctree& octree, const Submesh& targetSubmesh, uint32_t& outTriangleIndex);
    bool Intersects(const Ray& ray, const Submesh& submesh, Submesh::IntersectInfo& outIntersectInfo);
    bool Intersects(const Ray& ray, const MeshRegistry& meshRegistry, MeshRegistry::IntersectInfo& outIntersectInfo);
}
