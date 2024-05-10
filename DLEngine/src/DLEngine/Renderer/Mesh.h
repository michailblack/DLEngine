#pragma once
#include "DLEngine/Math/Primitives.h"
#include "DLEngine/Math/Mat4x4.h"
#include "DLEngine/Math/Vec2.h"
#include "DLEngine/Math/Vec3.h"

namespace DLEngine
{
    class Model;

    class Mesh
    {
        friend class Model;
    public:
        struct Vertex
        {
            Math::Vec3 Position;
            Math::Vec3 Normal;
            Math::Vec3 Tangent;
            Math::Vec3 Bitangent;
            Math::Vec2 TexCoords;
        };

        struct Triangle
        {
            uint32_t Indices[3];
        };

        struct Instance
        {
            Ref<Model> Model;
            uint32_t MeshIndex{ 0u };
            uint32_t MeshInstanceIndex{ 0u };
        };

        struct IntersectInfo
        {
            Instance MeshInstance;
            Math::Vec3 IntersectionPoint;
            Math::Vec3 Normal;
            float T{ Math::Infinity() };
        };

    public:
        uint32_t GetInstanceCount() const noexcept { return static_cast<uint32_t>(m_Instances.size()); }

        Math::Mat4x4& GetInstance(uint32_t instanceIndex) noexcept { return m_Instances[instanceIndex]; }
        Math::Mat4x4& GetInvInstance(uint32_t instanceIndex) noexcept { return m_InvInstances[instanceIndex]; }

        const Math::Mat4x4& GetInstance(uint32_t instanceIndex) const noexcept { return m_Instances[instanceIndex]; }
        const Math::Mat4x4& GetInvInstance(uint32_t instanceIndex) const noexcept { return m_InvInstances[instanceIndex]; }

    private:
        std::vector<Vertex> m_Vertices;
        std::vector<Triangle> m_Triangles;
        std::vector<Math::Mat4x4> m_Instances;
        std::vector<Math::Mat4x4> m_InvInstances;

        std::string m_Name;
        Math::AABB m_BoundingBox;
    };
}
