#include "dlpch.h"
#include "VertexBuffer.h"

namespace DLEngine
{
    namespace Utils
    {
        namespace
        {
            uint32_t ShaderDataTypeSize(BufferLayout::ShaderDataType type)
            {
                switch (type)
                {
                case BufferLayout::ShaderDataType::Float:   return sizeof(float);
                case BufferLayout::ShaderDataType::Float2:  return sizeof(float) * 2;
                case BufferLayout::ShaderDataType::Float3:  return sizeof(float) * 3;
                case BufferLayout::ShaderDataType::Float4:  return sizeof(float) * 4;
                case BufferLayout::ShaderDataType::Mat4:    return sizeof(float) * 4 * 4;
                default: DL_ASSERT(false, "Unknown ShaderDatatType!"); return 0;
                }
            }
        }
    }

    void VertexBuffer::Bind(uint32_t slot)
    {
        static constexpr uint32_t offset{ 0u };

        D3D::GetDeviceContext4()->IASetVertexBuffers(slot, 1u, m_VertexBuffer.GetAddressOf(), &m_Stride, &offset);
    }

    uint32_t VertexBuffer::CalculateStride() const noexcept
    {
        uint32_t stride{ 0u };
        for (const auto& bufferElement : m_BufferLayout)
            stride += Utils::ShaderDataTypeSize(bufferElement.DataType);

        return stride;
    }
}
