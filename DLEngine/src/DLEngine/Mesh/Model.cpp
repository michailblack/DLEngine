#include "dlpch.h"
#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#ifdef DL_DEBUG
#pragma comment(lib, "assimp-vc143-mtd.lib")
#else
#pragma comment(lib, "assimp-vc143-mt.lib")
#endif

namespace DLEngine
{
    Model::Model(const std::string& path)
        : m_Name(path)
    {
        static Assimp::Importer s_Importer;

        uint32_t importFlags{
            aiProcess_Triangulate |
            aiProcess_GenBoundingBoxes |
            static_cast<uint32_t>(aiProcess_ConvertToLeftHanded) |
            aiProcess_CalcTangentSpace
        };

        const aiScene* assimpScene{ s_Importer.ReadFile(path, importFlags) };

        DL_ASSERT(assimpScene, "Failed to load model '{}'", path);
        DL_ASSERT(assimpScene->mRootNode, "Failed to load model '{}'", path);
        DL_ASSERT(!(assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "Failed to complete scene flags for model '{}'", path);

        static_assert(sizeof(Math::Vec3) == sizeof(aiVector3D));
        static_assert(sizeof(Math::Vec2) == sizeof(aiVector2D));
        static_assert(sizeof(Mesh::Triangle) == 3u * sizeof(uint32_t));

        m_Meshes.resize(assimpScene->mNumMeshes);
        m_Ranges.resize(assimpScene->mNumMeshes);

        std::vector<Mesh::Vertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t i{ 0u }; i < assimpScene->mNumMeshes; ++i)
        {
            auto& srcMesh{ assimpScene->mMeshes[i] };
            auto& dstMesh{ m_Meshes[i] };

            dstMesh.m_Name = srcMesh->mName.C_Str();
            dstMesh.m_BoundingBox.Min = reinterpret_cast<Math::Vec3&>(srcMesh->mAABB.mMin);
            dstMesh.m_BoundingBox.Max = reinterpret_cast<Math::Vec3&>(srcMesh->mAABB.mMax);

            dstMesh.m_Vertices.resize(srcMesh->mNumVertices);
            dstMesh.m_Triangles.resize(srcMesh->mNumFaces);

            vertices.reserve(vertices.size() + srcMesh->mNumVertices);
            indices.reserve(indices.size() + srcMesh->mNumFaces * 3u);

            m_Ranges[i].VertexOffset = static_cast<uint32_t>(vertices.size());
            m_Ranges[i].VertexCount = srcMesh->mNumVertices;

            for (uint32_t v{ 0u }; v < srcMesh->mNumVertices; ++v)
            {
                Mesh::Vertex& vertex{ dstMesh.m_Vertices[v] };

                if (srcMesh->HasPositions())
                    vertex.Position = reinterpret_cast<Math::Vec3&>(srcMesh->mVertices[v]);
                if (srcMesh->HasNormals())
                    vertex.Normal = reinterpret_cast<Math::Vec3&>(srcMesh->mNormals[v]);
                if (srcMesh->HasTextureCoords(0))
                {
                    vertex.Tangent = reinterpret_cast<Math::Vec3&>(srcMesh->mTangents[v]);
                    vertex.Bitangent = reinterpret_cast<Math::Vec3&>(srcMesh->mBitangents[v]) * -1.0f;
                    vertex.TexCoords = reinterpret_cast<Math::Vec2&>(srcMesh->mTextureCoords[0][v]);
                }

                vertices.push_back(vertex);
            }

            m_Ranges[i].IndexOffset = static_cast<uint32_t>(indices.size());
            m_Ranges[i].IndexCount = srcMesh->mNumFaces * 3u;

            for (uint32_t f{ 0u }; f < srcMesh->mNumFaces; ++f)
            {
                const auto& face{ srcMesh->mFaces[f] };

                DL_ASSERT(face.mNumIndices == 3u, "Unsupported topology");
                
                Mesh::Triangle& triangle{ dstMesh.m_Triangles[f] };
                triangle = *reinterpret_cast<Mesh::Triangle*>(face.mIndices);

                indices.push_back(triangle.Indices[0]);
                indices.push_back(triangle.Indices[1]);
                indices.push_back(triangle.Indices[2]);
            }

            dstMesh.UpdateOctree();
        }

        m_VertexBuffer = CreateScope<PerVertexBuffer<Mesh::Vertex>>(GetCommonVertexBufferLayout(), vertices);
        m_IndexBuffer = CreateScope<IndexBuffer>(indices);

        std::function<void(aiNode*)> loadInstances;
        loadInstances = [&loadInstances, this](const aiNode* node)
            {
                auto assimpNodeToParent{ node->mTransformation };
                const Math::Mat4x4 nodeToParent{ reinterpret_cast<const Math::Mat4x4&>(assimpNodeToParent.Transpose()) };
                const Math::Mat4x4 parentToNode{ Math::Mat4x4::Inverse(nodeToParent) };

                for (uint32_t i{ 0u }; i < node->mNumMeshes; ++i)
                {
                    uint32_t meshIndex{ node->mMeshes[i] };
                    m_Meshes[meshIndex].m_Instances.push_back(nodeToParent);
                    m_Meshes[meshIndex].m_InvInstances.push_back(parentToNode);
                }

                for (uint32_t i{ 0u }; i < node->mNumChildren; ++i)
                    loadInstances(node->mChildren[i]);
            };

        loadInstances(assimpScene->mRootNode);
    }

    bool Model::Intersects(const Math::Ray& ray, Model::IntersectInfo& outIntersectInfo) const noexcept
    {
        bool intersects{ false };
        for (uint32_t meshIndex{ 0u }; meshIndex < m_Meshes.size(); ++meshIndex)
        {
            const Mesh& mesh{ m_Meshes[meshIndex] };

            if (mesh.Intersects(ray, outIntersectInfo.MeshIntersectInfo))
            {
                outIntersectInfo.MeshIndex = meshIndex;
                intersects = true;
            }
        }
        return intersects;
    }

    Mesh& Model::GetMesh(uint32_t meshIndex) noexcept
    {
        DL_ASSERT_NOINFO(meshIndex < static_cast<uint32_t>(m_Meshes.size()));
        return m_Meshes[meshIndex];
    }

    const Mesh& Model::GetMesh(uint32_t meshIndex) const noexcept
    {
        DL_ASSERT_NOINFO(meshIndex < static_cast<uint32_t>(m_Meshes.size()));
        return m_Meshes[meshIndex];
    }

    const Model::MeshRange& Model::GetMeshRange(uint32_t meshIndex) const noexcept
    {
        DL_ASSERT_NOINFO(meshIndex < static_cast<uint32_t>(m_Ranges.size()));
        return m_Ranges[meshIndex];
    }

    BufferLayout Model::GetCommonVertexBufferLayout() noexcept
    {
        static const BufferLayout bufferLayout{
            { "POSITION"  , BufferLayout::ShaderDataType::Float3 },
            { "NORMAL"    , BufferLayout::ShaderDataType::Float3 },
            { "TANGENT"   , BufferLayout::ShaderDataType::Float3 },
            { "BITANGENT" , BufferLayout::ShaderDataType::Float3 },
            { "TEXCOORDS" , BufferLayout::ShaderDataType::Float2 }
        };

        return bufferLayout;
    }
}
