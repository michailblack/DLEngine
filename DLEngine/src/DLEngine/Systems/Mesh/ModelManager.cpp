#include "dlpch.h"
#include "ModelManager.h"

#include "DLEngine/Systems/Mesh/Model.h"

#include <numeric>

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
    namespace
    {
        struct
        {
            std::unordered_map<std::wstring, Ref<Model>> Models;
        } s_Data;
    }

    void ModelManager::Init()
    {
		InitUnitSphere();

		DL_LOG_INFO("ModelManager initialized");
    }

    Ref<Model> ModelManager::Load(const std::wstring& path)
    {
        if (Exists(path))
            return s_Data.Models[path];

        Ref<Model> model{ LoadFromFile(path) };

        s_Data.Models.emplace(std::make_pair(path, model));

        return model;
    }

    Ref<Model> ModelManager::Get(const std::wstring& path)
    {
        DL_ASSERT_NOINFO(Exists(path));

        return s_Data.Models[path];
    }

    bool ModelManager::Exists(const std::wstring& path)
    {
        return s_Data.Models.contains(path);
    }

    Ref<Model> ModelManager::LoadFromFile(const std::wstring& path)
    {
        static Assimp::Importer s_Importer;

        uint32_t importFlags{
            aiProcess_Triangulate |
		    aiProcess_GenBoundingBoxes |
		     static_cast<uint32_t>(aiProcess_ConvertToLeftHanded) |
		    aiProcess_CalcTangentSpace
        };

	    std::string pathStr{ Utils::WideStrToMultiByteStr(path) };

        const aiScene* assimpScene{ s_Importer.ReadFile(pathStr, importFlags) };

        DL_ASSERT(assimpScene, "Failed to load model '{}'", pathStr);
        DL_ASSERT(assimpScene->mRootNode, "Failed to load model '{}'", pathStr);
        DL_ASSERT(!(assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "Failed to complete scene flags for model '{}'", pathStr);

        static_assert(sizeof(Math::Vec3) == sizeof(aiVector3D));
        static_assert(sizeof(Math::Vec2) == sizeof(aiVector2D));
        static_assert(sizeof(Mesh::Triangle) == 3u * sizeof(uint32_t));

        Ref<Model> model{ CreateRef<Model>() };
        model->m_Name = path;

		model->m_Meshes.resize(assimpScene->mNumMeshes);
		model->m_Ranges.resize(assimpScene->mNumMeshes);

		std::vector<Mesh::Vertex> vertices;
		std::vector<uint32_t> indices;

        model->m_BoundingBox.Min = Math::Vec3{ Math::Numeric::Max };
        model->m_BoundingBox.Max = Math::Vec3{ -Math::Numeric::Max };

		for (uint32_t i{ 0u }; i < assimpScene->mNumMeshes; ++i)
		{
            auto& srcMesh{ assimpScene->mMeshes[i] };
            auto& dstMesh{ model->m_Meshes[i] };

            dstMesh.m_Name = srcMesh->mName.C_Str();
            dstMesh.m_BoundingBox.Min = reinterpret_cast<Math::Vec3&>(srcMesh->mAABB.mMin);
            dstMesh.m_BoundingBox.Max = reinterpret_cast<Math::Vec3&>(srcMesh->mAABB.mMax);

            dstMesh.m_Vertices.resize(srcMesh->mNumVertices);
            dstMesh.m_Triangles.resize(srcMesh->mNumFaces);

            vertices.reserve(vertices.size() + srcMesh->mNumVertices);
            indices.reserve(indices.size() + srcMesh->mNumFaces * 3u);

            model->m_Ranges[i].VertexOffset = static_cast<uint32_t>(vertices.size());
            model->m_Ranges[i].VertexCount = srcMesh->mNumVertices;

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

                model->m_BoundingBox.Min = Math::Min(model->m_BoundingBox.Min, vertex.Position);
                model->m_BoundingBox.Max = Math::Max(model->m_BoundingBox.Max, vertex.Position);
            }

            model->m_Ranges[i].IndexOffset = static_cast<uint32_t>(indices.size());
            model->m_Ranges[i].IndexCount = srcMesh->mNumFaces * 3u;

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

        model->m_VertexBuffer.SetBufferLayout(Model::GetCommonVertexBufferLayout());
        model->m_VertexBuffer.Resize(vertices);
        
        model->m_IndexBuffer.Create(indices);

        std::function<void(aiNode*)> loadInstances;
        loadInstances = [&loadInstances, &model](const aiNode* node)
            {
                auto assimpNodeToParent{ node->mTransformation };
                const Math::Mat4x4 nodeToParent{ reinterpret_cast<const Math::Mat4x4&>(assimpNodeToParent.Transpose()) };
                const Math::Mat4x4 parentToNode{ Math::Mat4x4::Inverse(nodeToParent) };

                for (uint32_t i{ 0u }; i < node->mNumMeshes; ++i)
                {
                    uint32_t meshIndex{ node->mMeshes[i] };
                    model->m_Meshes[meshIndex].m_Instances.push_back(nodeToParent);
                    model->m_Meshes[meshIndex].m_InvInstances.push_back(parentToNode);
                }

                for (uint32_t i{ 0u }; i < node->mNumChildren; ++i)
                    loadInstances(node->mChildren[i]);
            };

        loadInstances(assimpScene->mRootNode);

        return model;
    }

    void ModelManager::InitUnitSphere()
    {
	    constexpr uint32_t SIDES = 6;
	    constexpr uint32_t GRID_SIZE = 12u;
	    constexpr uint32_t TRIS_PER_SIDE = GRID_SIZE * GRID_SIZE * 2u;
	    constexpr uint32_t VERT_PER_SIZE = (GRID_SIZE + 1u) * (GRID_SIZE + 1u);

		Ref<Model> model{ CreateRef<Model>() };
	    model->m_Name = L"UNIT_SPHERE";
        model->m_BoundingBox.Min = Math::Vec3{ -1.0f, -1.0f, -1.0f };
        model->m_BoundingBox.Max = Math::Vec3{ 1.0f, 1.0f, 1.0f };

	    Mesh& mesh = model->m_Meshes.emplace_back();
	    mesh.m_Name = "UNIT_SPHERE";
	    mesh.m_Instances = { Math::Mat4x4::Identity() };
	    mesh.m_InvInstances = { Math::Mat4x4::Identity() };

	    mesh.m_Vertices.resize(VERT_PER_SIZE * SIDES);
	    Mesh::Vertex* vertex = mesh.m_Vertices.data();

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

				    vertex[0] = Mesh::Vertex{};

				    vertex[0].Position[sideMasks[side][0]] = v.x * sideSigns[side][0];
				    vertex[0].Position[sideMasks[side][1]] = v.y * sideSigns[side][1];
				    vertex[0].Position[sideMasks[side][2]] = v.z * sideSigns[side][2];
				    vertex[0].Normal = vertex[0].Position = Math::Normalize(vertex[0].Position);

				    vertex += 1;
			    }
		    }
	    }

	    mesh.m_Triangles.resize(TRIS_PER_SIDE * SIDES);
	    auto* triangle{ mesh.m_Triangles.data() };

		std::vector<uint32_t> indices{};
		indices.reserve(mesh.m_Triangles.size() * 3u);

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

		mesh.m_BoundingBox.Min = Math::Vec3{ -1.0f, -1.0f, -1.0f };
		mesh.m_BoundingBox.Max = Math::Vec3{ 1.0f, 1.0f, 1.0f };
		mesh.UpdateOctree();

		model->m_VertexBuffer.SetBufferLayout(Model::GetCommonVertexBufferLayout());
		model->m_VertexBuffer.Resize(mesh.m_Vertices);

		model->m_IndexBuffer.Create(indices);

		Model::MeshRange range{};
		range.VertexOffset = 0u;
		range.IndexOffset = 0u;
		range.VertexCount = static_cast<uint32_t>(mesh.m_Vertices.size());
		range.IndexCount = static_cast<uint32_t>(indices.size());

		model->m_Ranges.push_back(range);

		s_Data.Models.emplace(std::make_pair(model->m_Name, model));
    }
}
