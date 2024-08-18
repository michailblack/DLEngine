#include "dlpch.h"
#include "Mesh.h"

#include "DLEngine/Core/Application.h"

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

    Mesh::Mesh(const std::filesystem::path& path) noexcept
    {
        LoadFromFile(path);
    }

    VertexBufferLayout Mesh::GetCommonVertexBufferLayout() noexcept
    {
        static const VertexBufferLayout bufferLayout{
            { "POSITION"  , ShaderDataType::Float3 },
            { "NORMAL"    , ShaderDataType::Float3 },
            { "TANGENT"   , ShaderDataType::Float3 },
            { "BITANGENT" , ShaderDataType::Float3 },
            { "TEXCOORDS" , ShaderDataType::Float2 }
        };

        return bufferLayout;
    }

    const std::filesystem::path Mesh::GetMeshDirectoryPath() noexcept
    {
        return Application::Get().GetWorkingDir() / "assets\\models\\";
    }

    Ref<Mesh> Mesh::CreateUnitSphere()
    {
        constexpr uint32_t SIDES = 6;
        constexpr uint32_t GRID_SIZE = 12u;
        constexpr uint32_t TRIS_PER_SIDE = GRID_SIZE * GRID_SIZE * 2u;
        constexpr uint32_t VERT_PER_SIZE = (GRID_SIZE + 1u) * (GRID_SIZE + 1u);

        Ref<Mesh> mesh{ CreateRef<Mesh>() };
        mesh->m_Name = "UNIT_SPHERE";
        mesh->m_BoundingBox.Min = Math::Vec3{ -1.0f, -1.0f, -1.0f };
        mesh->m_BoundingBox.Max = Math::Vec3{ 1.0f, 1.0f, 1.0f };

        Submesh& submesh{ mesh->m_Submeshes.emplace_back() };
        submesh.m_Name = "UNIT_SPHERE";
        submesh.m_Instances = { Math::Mat4x4::Identity() };
        submesh.m_InvInstances = { Math::Mat4x4::Identity() };

        submesh.m_Vertices.resize(VERT_PER_SIZE * SIDES);
        Submesh::Vertex* vertex = submesh.m_Vertices.data();

        uint32_t sideMasks[6][3] =
        {
            { 2u, 1u, 0u },
            { 0u, 1u, 2u },
            { 2u, 1u, 0u },
            { 0u, 1u, 2u },
            { 0u, 2u, 1u },
            { 0u, 2u, 1u }
        };

        float sideSigns[6][3] =
        {
            { +1.0f, +1.0f, +1.0f },
            { -1.0f, +1.0f, +1.0f },
            { -1.0f, +1.0f, -1.0f },
            { +1.0f, +1.0f, -1.0f },
            { +1.0f, -1.0f, -1.0f },
            { +1.0f, +1.0f, +1.0f }
        };

        for (uint32_t side{ 0u }; side < SIDES; ++side)
        {
            for (uint32_t row{ 0u }; row < GRID_SIZE + 1; ++row)
            {
                for (uint32_t col{ 0u }; col < GRID_SIZE + 1; ++col)
                {
                    Math::Vec3 v{};
                    v.x = static_cast<float>(col) / static_cast<float>(GRID_SIZE) * 2.0f - 1.0f;
                    v.y = static_cast<float>(row) / static_cast<float>(GRID_SIZE) * 2.0f - 1.0f;
                    v.z = 1.0f;

                    vertex[0] = Submesh::Vertex{};

                    vertex[0].Position[sideMasks[side][0]] = v.x * sideSigns[side][0];
                    vertex[0].Position[sideMasks[side][1]] = v.y * sideSigns[side][1];
                    vertex[0].Position[sideMasks[side][2]] = v.z * sideSigns[side][2];
                    vertex[0].Normal = vertex[0].Position = Math::Normalize(vertex[0].Position);

                    vertex += 1;
                }
            }
        }

        submesh.m_Triangles.resize(TRIS_PER_SIDE * SIDES);
        auto* triangle{ submesh.m_Triangles.data() };

        std::vector<uint32_t> indices{};
        indices.reserve(submesh.m_Triangles.size() * 3u);

        for (uint32_t side{ 0u }; side < SIDES; ++side)
        {
            uint32_t sideOffset{ VERT_PER_SIZE * side };

            for (uint32_t row{ 0u }; row < GRID_SIZE; ++row)
            {
                for (uint32_t col{ 0u }; col < GRID_SIZE; ++col)
                {
                    triangle[0].Indices[0] = sideOffset + (row + 0u) * (GRID_SIZE + 1u) + col + 0u;
                    triangle[0].Indices[1] = sideOffset + (row + 1u) * (GRID_SIZE + 1u) + col + 0u;
                    triangle[0].Indices[2] = sideOffset + (row + 0u) * (GRID_SIZE + 1u) + col + 1u;

                    triangle[1].Indices[0] = sideOffset + (row + 1u) * (GRID_SIZE + 1u) + col + 0u;
                    triangle[1].Indices[1] = sideOffset + (row + 1u) * (GRID_SIZE + 1u) + col + 1u;
                    triangle[1].Indices[2] = sideOffset + (row + 0u) * (GRID_SIZE + 1u) + col + 1u;

                    indices.push_back(triangle[0].Indices[0]);
                    indices.push_back(triangle[0].Indices[1]);
                    indices.push_back(triangle[0].Indices[2]);

                    indices.push_back(triangle[1].Indices[0]);
                    indices.push_back(triangle[1].Indices[1]);
                    indices.push_back(triangle[1].Indices[2]);

                    triangle += 2;
                }
            }
        }

        submesh.m_BoundingBox.Min = Math::Vec3{ -1.0f, -1.0f, -1.0f };
        submesh.m_BoundingBox.Max = Math::Vec3{ 1.0f, 1.0f, 1.0f };
        submesh.UpdateOctree();

        mesh->m_VertexBuffer = VertexBuffer::Create(
            Mesh::GetCommonVertexBufferLayout(),
            Buffer{ submesh.m_Vertices.data(), submesh.m_Vertices.size() * sizeof(Submesh::Vertex) }
        );

        mesh->m_IndexBuffer = IndexBuffer::Create(Buffer{ indices.data(), indices.size() * sizeof(uint32_t) });

        Mesh::Range range{};
        range.VertexOffset = 0u;
        range.IndexOffset = 0u;
        range.VertexCount = static_cast<uint32_t>(submesh.m_Vertices.size());
        range.IndexCount = static_cast<uint32_t>(indices.size());

        mesh->m_Ranges.push_back(range);

        return mesh;
    }

    void Mesh::LoadFromFile(const std::filesystem::path& path)
    {
        static Assimp::Importer s_Importer;

        uint32_t importFlags{
            aiProcess_Triangulate |
            aiProcess_GenBoundingBoxes |
             static_cast<uint32_t>(aiProcess_ConvertToLeftHanded) |
            aiProcess_CalcTangentSpace
        };

        const aiScene* assimpScene{ s_Importer.ReadFile(path.string().c_str(), importFlags) };

        DL_ASSERT(assimpScene, "Failed to load model '{}'", path.string().c_str());
        DL_ASSERT(assimpScene->mRootNode, "Failed to load model '{}'", path.string().c_str());
        DL_ASSERT(!(assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "Failed to complete scene flags for model '{}'", path.string().c_str());

        static_assert(sizeof(Math::Vec3) == sizeof(aiVector3D));
        static_assert(sizeof(Math::Vec2) == sizeof(aiVector2D));
        static_assert(sizeof(Submesh::Triangle) == 3u * sizeof(uint32_t));

        m_Name = path.stem().string();

        m_Submeshes.resize(assimpScene->mNumMeshes);
        m_Ranges.resize(assimpScene->mNumMeshes);

        m_BoundingBox.Min = Math::Vec3{ Math::Numeric::Max };
        m_BoundingBox.Max = Math::Vec3{ -Math::Numeric::Max };

        std::vector<Submesh::Vertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t i{ 0u }; i < assimpScene->mNumMeshes; ++i)
        {
            auto& srcMesh{ assimpScene->mMeshes[i] };
            auto& dstMesh{ m_Submeshes[i] };

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
                Submesh::Vertex& vertex{ dstMesh.m_Vertices[v] };

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

                m_BoundingBox.Min = Math::Min(m_BoundingBox.Min, vertex.Position);
                m_BoundingBox.Max = Math::Max(m_BoundingBox.Max, vertex.Position);
            }

            m_Ranges[i].IndexOffset = static_cast<uint32_t>(indices.size());
            m_Ranges[i].IndexCount = srcMesh->mNumFaces * 3u;

            for (uint32_t f{ 0u }; f < srcMesh->mNumFaces; ++f)
            {
                const auto& face{ srcMesh->mFaces[f] };

                DL_ASSERT(face.mNumIndices == 3u, "Unsupported topology");

                Submesh::Triangle& triangle{ dstMesh.m_Triangles[f] };
                triangle = *reinterpret_cast<Submesh::Triangle*>(face.mIndices);

                indices.push_back(triangle.Indices[0]);
                indices.push_back(triangle.Indices[1]);
                indices.push_back(triangle.Indices[2]);
            }

            dstMesh.UpdateOctree();
        }

        m_VertexBuffer = VertexBuffer::Create(Mesh::GetCommonVertexBufferLayout(), Buffer{ vertices.data(), vertices.size() * sizeof(Submesh::Vertex) });
        m_IndexBuffer = IndexBuffer::Create(Buffer{ indices.data(), indices.size() * sizeof(uint32_t) });

        std::function<void(aiNode*)> loadInstances;
        loadInstances = [&loadInstances, this](const aiNode* node)
            {
                auto assimpNodeToParent{ node->mTransformation };
                const Math::Mat4x4 nodeToParent{ reinterpret_cast<const Math::Mat4x4&>(assimpNodeToParent.Transpose()) };
                const Math::Mat4x4 parentToNode{ Math::Mat4x4::Inverse(nodeToParent) };

                for (uint32_t i{ 0u }; i < node->mNumMeshes; ++i)
                {
                    uint32_t meshIndex{ node->mMeshes[i] };
                    m_Submeshes[meshIndex].m_Instances.push_back(nodeToParent);
                    m_Submeshes[meshIndex].m_InvInstances.push_back(parentToNode);
                }

                for (uint32_t i{ 0u }; i < node->mNumChildren; ++i)
                    loadInstances(node->mChildren[i]);
            };

        loadInstances(assimpScene->mRootNode);

        DL_LOG_INFO_TAG("Mesh", "Loaded mesh [{0}]", m_Name);
    }

    void MeshLibrary::Init()
    {
        Add(Mesh::CreateUnitSphere());
    }

    void MeshLibrary::Add(const Ref<Mesh>& mesh)
    {
        DL_ASSERT(!m_Meshes.contains(mesh->GetName()), "Mesh [{0}] already added in the mesh library", mesh->GetName());

        m_Meshes[mesh->GetName()] = mesh;
    }

    Ref<Mesh> MeshLibrary::Load(const std::filesystem::path& path)
    {
        Ref<Mesh> mesh{ CreateRef<Mesh>(path) };
        Add(mesh);

        return mesh;
    }

    Ref<Mesh> MeshLibrary::Get(const std::string& path)
    {
        DL_ASSERT(m_Meshes.contains(path), "Mesh [{0}] not found in the mesh library", path);

        return m_Meshes[path];
    }

}