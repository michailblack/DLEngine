#include "dlpch.h"
#include "Model.h"

namespace DLEngine
{
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
