#include "dlpch.h"
#include "VertexLayout.h"

namespace Utils
{
    namespace 
    {
        uint32_t GetDXGIFormatSize(DXGI_FORMAT format) noexcept
        {
            switch (format)
            {
                case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16u;
                case DXGI_FORMAT_R32G32B32_FLOAT:    return 12u;
                case DXGI_FORMAT_R32G32_FLOAT:       return 8u;
                case DXGI_FORMAT_R32_FLOAT:          return 4u;
                default: return 0u;
            }
        }
    }
}

void VertexLayout::Append(Element element) noexcept
{
    m_VertexLayoutElements.push_back(std::move(element));
}

uint32_t VertexLayout::GetVertexSize() const noexcept
{
    uint32_t size { 0u };
    for (const auto& element : m_VertexLayoutElements)
        size += Utils::GetDXGIFormatSize(element.Format);
    return size;
}

uint32_t VertexLayout::GetElementCount() const noexcept
{
    return static_cast<uint32_t>(m_VertexLayoutElements.size());
}

const std::vector<VertexLayout::Element>& VertexLayout::GetElements() const noexcept
{
    return m_VertexLayoutElements;
}
