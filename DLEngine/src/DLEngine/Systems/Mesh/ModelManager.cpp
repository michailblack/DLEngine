#include "dlpch.h"
#include "ModelManager.h"

#include "DLEngine/Systems/Mesh/Model.h"

#include <numeric>

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

        const auto& [it, hasConstructed]{ s_Data.Models.emplace(std::make_pair(path, CreateRef<Model>(path))) };

        return it->second;
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

    void ModelManager::InitUnitSphere()
    {
	    constexpr uint32_t SIDES = 6;
	    constexpr uint32_t GRID_SIZE = 12u;
	    constexpr uint32_t TRIS_PER_SIDE = GRID_SIZE * GRID_SIZE * 2u;
	    constexpr uint32_t VERT_PER_SIZE = (GRID_SIZE + 1u) * (GRID_SIZE + 1u);

		Ref<Model> model{ CreateRef<Model>() };
	    model->m_Name = L"UNIT_SPHERE";

	    Mesh& mesh = model->m_Meshes.emplace_back();
	    mesh.m_Name = "UNIT_SPHERE";
	    mesh.m_Instances = { Math::Mat4x4::Identity() };
	    mesh.m_InvInstances = { Math::Mat4x4::Identity() };

	    mesh.m_Vertices.resize(VERT_PER_SIZE * SIDES);
	    Mesh::Vertex* vertex = mesh.m_Vertices.data();

	    int sideMasks[6][3] =
	    {
		    { 2, 1, 0 },
		    { 0, 1, 2 },
		    { 2, 1, 0 },
		    { 0, 1, 2 },
		    { 0, 2, 1 },
		    { 0, 2, 1 }
	    };

	    float sideSigns[6][3] =
	    {
		    { +1, +1, +1 },
		    { -1, +1, +1 },
		    { -1, +1, -1 },
		    { +1, +1, -1 },
		    { +1, -1, -1 },
		    { +1, +1, +1 }
	    };

	    for (int side = 0; side < SIDES; ++side)
	    {
		    for (int row = 0; row < GRID_SIZE + 1; ++row)
		    {
			    for (int col = 0; col < GRID_SIZE + 1; ++col)
			    {
				    Math::Vec3 v{};
				    v.x = static_cast<float>(col) / static_cast<float>(GRID_SIZE) * 2.f - 1.f;
				    v.y = static_cast<float>(row) / static_cast<float>(GRID_SIZE) * 2.f - 1.f;
				    v.z = 1.f;

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
	    auto* triangle = mesh.m_Triangles.data();

		std::vector<uint32_t> indices{};
		indices.reserve(mesh.m_Triangles.size() * 3u);

	    for (int side = 0; side < SIDES; ++side)
	    {
		    uint32_t sideOffset = VERT_PER_SIZE * side;

		    for (int row = 0; row < GRID_SIZE; ++row)
		    {
			    for (int col = 0; col < GRID_SIZE; ++col)
			    {
				    triangle[0].Indices[0] = sideOffset + (row + 0) * (GRID_SIZE + 1) + col + 0;
				    triangle[0].Indices[1] = sideOffset + (row + 1) * (GRID_SIZE + 1) + col + 0;
				    triangle[0].Indices[2] = sideOffset + (row + 0) * (GRID_SIZE + 1) + col + 1;

				    triangle[1].Indices[0] = sideOffset + (row + 1) * (GRID_SIZE + 1) + col + 0;
				    triangle[1].Indices[1] = sideOffset + (row + 1) * (GRID_SIZE + 1) + col + 1;
				    triangle[1].Indices[2] = sideOffset + (row + 0) * (GRID_SIZE + 1) + col + 1;

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
		mesh.m_Octree.Build();

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
