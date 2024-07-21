#include "dlpch.h"
#include "BufferLayout.h"

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
                case BufferLayout::ShaderDataType::Int:     return sizeof(int32_t);
                case BufferLayout::ShaderDataType::Uint:    return sizeof(uint32_t);
                default: DL_ASSERT(false, "Unknown ShaderDatatType!"); return 0;
                }
            }
        }
    }

    BufferLayout::BufferLayout(const std::initializer_list<Element>& elements)
        : m_Elements(elements)
    {
        CalculateStride();
    }

    void BufferLayout::CalculateStride() noexcept
    {
        m_Stride = 0u;
        for (const auto& bufferElement : m_Elements)
            m_Stride += Utils::ShaderDataTypeSize(bufferElement.DataType);
    }
}
