#pragma once
#include "DLEngine/Core/DLWin.h"
#include "DLEngine/Core/DLException.h"

#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/BufferLayout.h"

#include <d3d11_4.h>
#include <wrl.h>
#include <dxgiformat.h>

namespace DLEngine
{
    class VertexBuffer
    {
    public:

        VertexBuffer(BufferLayout bufferLayout)
            : m_BufferLayout(std::move(bufferLayout))
        {
            m_Stride = CalculateStride();
        }

        virtual ~VertexBuffer() = default;

        void Bind(uint32_t slot = 0u);

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
        PerInstanceBuffer(BufferLayout bufferLayout, uint32_t instancesCount = 1u)
            : VertexBuffer(bufferLayout)
        {
            Resize(instancesCount);
        }

        void Resize(uint32_t instancesCount)
        {
            if (instancesCount == m_InstancesCount)
                return;

            m_InstancesCount = instancesCount;

            D3D11_BUFFER_DESC instanceBufferDesc{};
            instanceBufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(Vertex)) * instancesCount;
            instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            instanceBufferDesc.MiscFlags = 0u;
            instanceBufferDesc.StructureByteStride = m_Stride;

            DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&instanceBufferDesc, nullptr, &m_VertexBuffer));
        }

        Vertex* Map() const
        {
            D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
            DL_THROW_IF_HR(D3D::GetDeviceContext4()->Map(m_VertexBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource));
            return reinterpret_cast<Vertex*>(mappedSubresource.pData);
        }

        void Unmap() const
        {
            D3D::GetDeviceContext4()->Unmap(m_VertexBuffer.Get(), 0u);
        }

        ~PerInstanceBuffer() override = default;

    private:
        uint32_t m_InstancesCount;
    };
}
