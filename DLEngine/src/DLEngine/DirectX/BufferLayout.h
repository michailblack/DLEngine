#pragma once

namespace DLEngine
{
    class BufferLayout
    {
    public:
        enum class ShaderDataType
        {
            None = 0,
            Float, Float2, Float3, Float4,
            Mat4,
            Int, Uint
        };

        struct Element
        {
            std::string Semantics;
            ShaderDataType DataType;
        };

    public:
        BufferLayout(const std::initializer_list<Element>& elements);
        ~BufferLayout() = default;

        BufferLayout(const BufferLayout&) = default;
        BufferLayout(BufferLayout&&) noexcept = default;
        BufferLayout& operator=(const BufferLayout&) = default;
        BufferLayout& operator=(BufferLayout&&) noexcept = default;

        const std::vector<Element>& GetElements() const noexcept { return m_Elements; }
        uint32_t GetStride() const noexcept { return m_Stride; }

        std::vector<Element>::iterator begin() { return m_Elements.begin(); }
        std::vector<Element>::iterator end() { return m_Elements.end(); }

        std::vector<Element>::const_iterator begin() const { return m_Elements.begin(); }
        std::vector<Element>::const_iterator end() const { return m_Elements.end(); }

    private:
        void CalculateStride() noexcept;

    private:
        std::vector<Element> m_Elements{};
        uint32_t m_Stride{ 0u };
    };
}
