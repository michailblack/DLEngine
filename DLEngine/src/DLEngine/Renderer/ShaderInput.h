#pragma once
#include <string>
#include <vector>

namespace DLEngine
{
    enum class ShaderDataType
    {
        None = 0,
        Float, Float2, Float3, Float4,
        Mat3, Mat4,
        Int, Int2, Int3, Int4,
        Uint, Uint2, Uint3, Uint4,
        Bool
    };

    namespace Utils
    {
        inline uint32_t ShaderDataTypeSize(ShaderDataType type)
        {
            switch (type)
            {
            case ShaderDataType::Float:
            case ShaderDataType::Int:
            case ShaderDataType::Uint:
            case ShaderDataType::Bool:   return 4u;

            case ShaderDataType::Float2:
            case ShaderDataType::Int2:
            case ShaderDataType::Uint2:  return 4u * 2u;

            case ShaderDataType::Float3:
            case ShaderDataType::Int3:
            case ShaderDataType::Uint3:  return 4u * 3u;

            case ShaderDataType::Float4:
            case ShaderDataType::Int4:
            case ShaderDataType::Uint4:  return 4u * 4u;

            case ShaderDataType::Mat3:   return 3u * 3u * 4u;
            case ShaderDataType::Mat4:   return 4u * 4u * 4u;

            case ShaderDataType::None:
            default: DL_ASSERT(false, "Unknown ShaderDataType!"); return 0u;
            }
        }
    }

    struct VertexBufferElement
    {
        std::string Name;
        size_t Size;
        size_t Offset;
        ShaderDataType Type;

        VertexBufferElement() = default;
        VertexBufferElement(const std::string& name, ShaderDataType type)
            : Name(name), Size(0u), Offset(0u), Type(type)
        {}
    };

    class VertexBufferLayout
    {
    public:
        VertexBufferLayout(const std::initializer_list<VertexBufferElement>& elements)
            : m_Elements(elements)
        {
            CalculateStrideAndOffsets();
        }

        size_t GetStride() const noexcept { return m_Stride; }
        const std::vector<VertexBufferElement>& GetElements() const noexcept { return m_Elements; }
        uint32_t GetElementCount() const noexcept { return static_cast<uint32_t>(m_Elements.size()); }

        [[nodiscard]] std::vector<VertexBufferElement>::iterator begin() noexcept { return m_Elements.begin(); }
        [[nodiscard]] std::vector<VertexBufferElement>::iterator end() noexcept { return m_Elements.end(); }
        [[nodiscard]] std::vector<VertexBufferElement>::const_iterator begin() const noexcept { return m_Elements.begin(); }
        [[nodiscard]] std::vector<VertexBufferElement>::const_iterator end() const noexcept { return m_Elements.end(); }

    private:
        void CalculateStrideAndOffsets()
        {
            m_Stride = 0u;
            for (auto& element : m_Elements)
            {
                element.Size = Utils::ShaderDataTypeSize(element.Type);
                element.Offset = m_Stride;
                m_Stride += element.Size;
            }
        }

    private:
        std::vector<VertexBufferElement> m_Elements;
        size_t m_Stride{ 0u };
    };
}