#pragma once
#include <dxgiformat.h>

class VertexLayout
{
public:
    struct Element
    {
        std::string Semantics;
        DXGI_FORMAT Format;
        uint32_t Slot;
    };

    void Append(Element element) noexcept;

    uint32_t GetVertexSize() const noexcept;
    uint32_t GetElementCount() const noexcept;
    const std::vector<Element>& GetElements() const noexcept;

private:
    std::vector<Element> m_VertexLayoutElements;
};
