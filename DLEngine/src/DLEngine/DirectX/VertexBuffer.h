#pragma once
#include <dxgiformat.h>

#include "DLEngine/Core/DLException.h"

#include "DLEngine/DirectX/IBindable.h"
#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/BufferLayout.h"

namespace DLEngine
{
    class VertexBuffer
        : public IBindable
    {
    public:

        VertexBuffer(BufferLayout bufferLayout)
            : m_BufferLayout(std::move(bufferLayout))
        {
            m_Stride = CalculateStride();
        }

        ~VertexBuffer() override = default;

        void Bind() override;

        const BufferLayout& GetBufferLayout() const noexcept { return m_BufferLayout; }
        const uint32_t GetStride() const noexcept { return m_Stride; }

    private:
        uint32_t CalculateStride() const noexcept;

    protected:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
        uint32_t m_Stride{ 0u };

    private:
        BufferLayout m_BufferLayout;
    };


    template <typename Vertex>
    class PerVertexBuffer
        : public VertexBuffer
    {
    public:
        PerVertexBuffer(BufferLayout bufferLayout, const std::vector<Vertex> vertices)
            : VertexBuffer(bufferLayout)
        {
            D3D11_BUFFER_DESC vertexBufferDesc{};
            vertexBufferDesc.ByteWidth = m_Stride * static_cast<uint32_t>(vertices.size());
            vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vertexBufferDesc.CPUAccessFlags = 0u;
            vertexBufferDesc.MiscFlags = 0u;
            vertexBufferDesc.StructureByteStride = m_Stride;

            D3D11_SUBRESOURCE_DATA vertexBufferData{};
            vertexBufferData.pSysMem = vertices.data();
            vertexBufferData.SysMemPitch = 0u;
            vertexBufferData.SysMemSlicePitch = 0u;

            DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_VertexBuffer));
        }

        ~PerVertexBuffer() override = default;
    };


    template <typename Vertex>
    class PerInstanceBuffer
        : public VertexBuffer
    {
    public:
        PerInstanceBuffer(BufferLayout bufferLayout, const std::vector<Vertex> vertices)
            : VertexBuffer(bufferLayout)
            , m_BufferSize(m_Stride * vertices.size())
        {
            D3D11_BUFFER_DESC vertexBufferDesc{};
            vertexBufferDesc.ByteWidth = static_cast<uint32_t>(m_BufferSize);
            vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            vertexBufferDesc.MiscFlags = 0u;
            vertexBufferDesc.StructureByteStride = m_Stride;

            D3D11_SUBRESOURCE_DATA vertexBufferData{};
            vertexBufferData.pSysMem = vertices.data();
            vertexBufferData.SysMemPitch = 0u;
            vertexBufferData.SysMemSlicePitch = 0u;

            DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_VertexBuffer));
        }

        void Set(const std::vector<Vertex> vertices) const
        {
            DL_ASSERT_NOINFO(vertices.size() == m_BufferSize);

            D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
            DL_THROW_IF_HR(D3D::GetDeviceContext4()->Map(m_VertexBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource));
            memcpy_s(mappedSubresource.pData, m_BufferSize, vertices.data(), vertices.size() * sizeof(Vertex));
            D3D::GetDeviceContext4()->Unmap(m_VertexBuffer.Get(), 0u);
        }

        ~PerInstanceBuffer() override = default;

    private:
        size_t m_BufferSize;
    };
}
