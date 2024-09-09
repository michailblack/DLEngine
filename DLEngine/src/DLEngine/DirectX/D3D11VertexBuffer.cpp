#include "dlpch.h"
#include "D3D11VertexBuffer.h"

#include "DLEngine/DirectX/D3D11Context.h"

namespace DLEngine
{
    D3D11VertexBuffer::D3D11VertexBuffer(const VertexBufferLayout& layout, const Buffer& buffer, VertexBufferUsage usage)
        : m_Layout(layout), m_Size(buffer.Size), m_Usage(usage)
    {
        DL_ASSERT(layout.GetStride() > 0u, "Buffer layout must be set");
        DL_ASSERT(buffer, "Buffer must be valid");

        D3D11_BUFFER_DESC vertexBufferDesc{};
        vertexBufferDesc.ByteWidth = static_cast<UINT>(buffer.Size);
        vertexBufferDesc.Usage = usage == VertexBufferUsage::Static ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = usage == VertexBufferUsage::Static ? 0u : D3D11_CPU_ACCESS_WRITE;
        vertexBufferDesc.MiscFlags = 0u;
        vertexBufferDesc.StructureByteStride = static_cast<UINT>(layout.GetStride());

        D3D11_SUBRESOURCE_DATA vertexBufferData{};
        vertexBufferData.pSysMem = buffer.Data;
        vertexBufferData.SysMemPitch = 0u;
        vertexBufferData.SysMemSlicePitch = 0u;


        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_D3D11VertexBuffer));
    }

    D3D11VertexBuffer::D3D11VertexBuffer(const VertexBufferLayout& layout, size_t size, VertexBufferUsage usage)
        : m_Layout(layout), m_Size(size), m_Usage(usage)
    {
        DL_ASSERT(layout.GetStride() > 0u, "Buffer layout must be set");
        DL_ASSERT(size > 0u, "Buffer size must be greater than 0");
        DL_ASSERT(usage == VertexBufferUsage::Dynamic, "Static buffer must be initialized with data");

        D3D11_BUFFER_DESC vertexBufferDesc{};
        vertexBufferDesc.ByteWidth = static_cast<UINT>(size);
        vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        vertexBufferDesc.MiscFlags = 0u;
        vertexBufferDesc.StructureByteStride = static_cast<UINT>(layout.GetStride());

        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateBuffer(&vertexBufferDesc, nullptr, &m_D3D11VertexBuffer));
    }

    Buffer D3D11VertexBuffer::Map()
    {
        DL_ASSERT(m_Usage == VertexBufferUsage::Dynamic, "Vertex buffer must be dynamic to map data");

        D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDeviceContext4()->Map(
            m_D3D11VertexBuffer.Get(),
            0u,
            D3D11_MAP_WRITE_DISCARD,
            0u,
            &mappedSubresource
        ));

        return Buffer{ mappedSubresource.pData, m_Size };
    }

    void D3D11VertexBuffer::Unmap()
    {
        DL_ASSERT(m_Usage == VertexBufferUsage::Dynamic, "Vertex buffer must be dynamic to unmap data");

        D3D11Context::Get()->GetDeviceContext4()->Unmap(m_D3D11VertexBuffer.Get(), 0u);
    }

}
