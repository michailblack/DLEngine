#include "dlpch.h"
#include "VertexBuffer.h"

#include "DLEngine/DirectX/D3D.h"

VertexBuffer::VertexBuffer(const VertexLayout& vertexLayout, const void* data, uint32_t vertexNum)
    : m_VertexLayout(vertexLayout)
    , m_Stride(vertexLayout.GetVertexSize())
{
    D3D11_BUFFER_DESC vertexBufferDesc {};
    vertexBufferDesc.ByteWidth = m_Stride * vertexNum;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0u;
    vertexBufferDesc.MiscFlags = 0u;
    vertexBufferDesc.StructureByteStride = m_Stride;

    D3D11_SUBRESOURCE_DATA vertexBufferData {};
    vertexBufferData.pSysMem = data;
    vertexBufferData.SysMemPitch = 0u;
    vertexBufferData.SysMemSlicePitch = 0u;

    DL_THROW_IF(D3D::Get().GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_VertexBuffer));
}

void VertexBuffer::Bind()
{
    static constexpr uint32_t offset { 0u };
    D3D::Get().GetDeviceContext()->IASetVertexBuffers(0u, 1u, m_VertexBuffer.GetAddressOf(), &m_Stride, &offset);
}
