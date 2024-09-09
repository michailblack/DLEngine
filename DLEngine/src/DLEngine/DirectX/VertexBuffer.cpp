#include "dlpch.h"
#include "VertexBuffer.h"

namespace DLEngine
{
    void VertexBuffer::Create(const BufferLayout& layout, D3D11_USAGE usage, uint32_t vertexCount, const void* data)
    {
        DL_ASSERT(layout.GetStride() > 0u, "Buffer layout must be set");
        DL_ASSERT(vertexCount > 0u, "Vertex buffer can't be empty");

        m_BufferLayout = layout;
        m_VertexCount = vertexCount;

        D3D11_BUFFER_DESC vertexBufferDesc{};

        switch (usage)
        {
        case D3D11_USAGE_IMMUTABLE:
            {
                DL_ASSERT(data, "Immutable vertex buffer must be inited upon creation");

                vertexBufferDesc.ByteWidth = m_BufferLayout.GetStride() * vertexCount;
                vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
                vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                vertexBufferDesc.CPUAccessFlags = 0u;
                vertexBufferDesc.MiscFlags = 0u;
                vertexBufferDesc.StructureByteStride = m_BufferLayout.GetStride();
            } break;
        case D3D11_USAGE_DYNAMIC:
            {
                vertexBufferDesc.ByteWidth = m_BufferLayout.GetStride() * vertexCount;
                vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
                vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                vertexBufferDesc.MiscFlags = 0u;
                vertexBufferDesc.StructureByteStride = m_BufferLayout.GetStride();
            } break;
        case D3D11_USAGE_DEFAULT:
        case D3D11_USAGE_STAGING:
            {
                DL_ASSERT(false, "Unsupported usage type");
            } return;
        }

        D3D11_SUBRESOURCE_DATA vertexBufferData{};
        vertexBufferData.pSysMem = data;
        vertexBufferData.SysMemPitch = 0u;
        vertexBufferData.SysMemSlicePitch = 0u;

        D3D11_SUBRESOURCE_DATA* pVertexBufferData = data ? &vertexBufferData : nullptr;

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&vertexBufferDesc, pVertexBufferData, &m_Handle));
    }

    void VertexBuffer::Resize(uint32_t vertexCount)
    {
        DL_ASSERT(m_Handle && m_BufferLayout.GetStride() > 0u, "Vertex buffer is not created");

        D3D11_BUFFER_DESC vertexBufferDesk{};
        m_Handle->GetDesc(&vertexBufferDesk);

        DL_ASSERT(vertexBufferDesk.Usage != D3D11_USAGE_IMMUTABLE, "Can't resize immutable vertex buffer");

        if (m_VertexCount == vertexCount)
            return;

        m_Handle.Reset();

        Create(m_BufferLayout, D3D11_USAGE_DYNAMIC, vertexCount);
    }

    void* VertexBuffer::Map() const
    {
        D3D11_BUFFER_DESC vertexBufferDesk{};
        m_Handle->GetDesc(&vertexBufferDesk);

        DL_ASSERT(vertexBufferDesk.Usage != D3D11_USAGE_IMMUTABLE, "Can't map immutable vertex buffer");

        D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
        DL_THROW_IF_HR(D3D::GetDeviceContext4()->Map(
            m_Handle.Get(),
            0u,
            D3D11_MAP_WRITE_DISCARD,
            0u,
            &mappedSubresource
        ));

        return mappedSubresource.pData;
    }

    void VertexBuffer::Unmap() const
    {
        D3D::GetDeviceContext4()->Unmap(m_Handle.Get(), 0u);
    }
}
