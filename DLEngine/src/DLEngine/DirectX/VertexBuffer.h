#pragma once
#include "DLEngine/Core/DLException.h"

#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/BufferLayout.h"

namespace DLEngine
{
    enum class VertexBufferUsage
    {
        Immutable,
        Dynamic,
    };

    template <typename Vertex>
    class VertexBufferBase
    {
    public:
        void SetBufferLayout(const BufferLayout& bufferLayout) noexcept { m_BufferLayout = bufferLayout; }
        const BufferLayout& GetBufferLayout() const noexcept { return m_BufferLayout; }

        void Bind(uint32_t slot = 0u) const
        {
            uint32_t stride{ m_BufferLayout.GetStride() };
            uint32_t offset{ 0u };
            D3D::GetDeviceContext4()->IASetVertexBuffers(
                slot,
                1u,
                m_VertexBuffer.GetAddressOf(),
                &stride,
                &offset
            );
        }

    protected:
        VertexBufferBase() {}

    protected:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer{ nullptr };
        BufferLayout m_BufferLayout{};
    };

    template <typename Vertex, VertexBufferUsage Usage>
    class VertexBuffer
        : public VertexBufferBase<Vertex>
    {};

    template <typename Vertex>
    class VertexBuffer<Vertex, VertexBufferUsage::Immutable>
        : public VertexBufferBase<Vertex>
    {
        using VertexBufferBase<Vertex>::VertexBufferBase;
        using VertexBufferBase<Vertex>::m_BufferLayout;
        using VertexBufferBase<Vertex>::m_VertexBuffer;
    public:
        VertexBuffer()
            : VertexBufferBase<Vertex>{}
        {}

        void Create(const std::vector<Vertex>& vertices)
        {
            DL_ASSERT(m_BufferLayout.GetStride() > 0u, "You must set buffer layout first");
            DL_ASSERT(!vertices.empty(), "There must be at least 1 vertex in the Vertex Buffer");

            D3D11_BUFFER_DESC vertexBufferDesc{};
            vertexBufferDesc.ByteWidth = m_BufferLayout.GetStride() * static_cast<uint32_t>(vertices.size());
            vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vertexBufferDesc.CPUAccessFlags = 0u;
            vertexBufferDesc.MiscFlags = 0u;
            vertexBufferDesc.StructureByteStride = m_BufferLayout.GetStride();

            D3D11_SUBRESOURCE_DATA vertexBufferData{};
            vertexBufferData.pSysMem = vertices.data();
            vertexBufferData.SysMemPitch = 0u;
            vertexBufferData.SysMemSlicePitch = 0u;

            DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_VertexBuffer));
        }
    };

    template <typename Vertex>
    class VertexBuffer<Vertex, VertexBufferUsage::Dynamic>
        : public VertexBufferBase<Vertex>
    {
        using VertexBufferBase<Vertex>::VertexBufferBase;
        using VertexBufferBase<Vertex>::m_BufferLayout;
        using VertexBufferBase<Vertex>::m_VertexBuffer;
    public:
        VertexBuffer()
            : VertexBufferBase<Vertex>{}
        {}

        void Create(uint32_t vertexCount)
        {
            DL_ASSERT(m_BufferLayout.GetStride() > 0u, "You must set buffer layout first");
            DL_ASSERT(vertexCount > 0, "Vertex count must be greater than 0");

            if (m_VertexCount == vertexCount)
                return;

            m_VertexCount = vertexCount;

            m_VertexBuffer.Reset();

            D3D11_BUFFER_DESC vertexBufferDesc{};
            vertexBufferDesc.ByteWidth = m_BufferLayout.GetStride() * m_VertexCount;
            vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            vertexBufferDesc.MiscFlags = 0u;
            vertexBufferDesc.StructureByteStride = m_BufferLayout.GetStride();

            DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&vertexBufferDesc, nullptr, &m_VertexBuffer));
        }

        Vertex* Map() const
        {
            D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
            DL_THROW_IF_HR(D3D::GetDeviceContext4()->Map(
                m_VertexBuffer.Get(),
                0u,
                D3D11_MAP_WRITE_DISCARD,
                0u,
                &mappedSubresource
            ));
            return reinterpret_cast<Vertex*>(mappedSubresource.pData);
        }

        void Unmap() const
        {
            D3D::GetDeviceContext4()->Unmap(m_VertexBuffer.Get(), 0u);
        }

    private:
        uint32_t m_VertexCount{ 0u };
    };
}
