#pragma once
#include "DLEngine/Math/Primitives.h"
#include "DLEngine/Math/Vec2.h"
#include "DLEngine/Math/Vec3.h"

namespace DLEngine
{
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

    public:


    private:
        std::vector<Vertex> m_Vertices;
        std::vector<Triangle> m_Triangles;

        std::string m_Name;
        Math::AABB m_BoundingBox;
    };
}
