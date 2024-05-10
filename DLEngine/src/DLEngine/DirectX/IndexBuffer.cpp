#include "dlpch.h"
#include "IndexBuffer.h"

#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    IndexBuffer::IndexBuffer(const std::vector<uint32_t>& indices)
{
        D3D11_BUFFER_DESC indexBufferDesc{};
        indexBufferDesc.ByteWidth = sizeof(uint32_t) * static_cast<uint32_t>(indices.size());
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0u;
        indexBufferDesc.MiscFlags = 0u;
        indexBufferDesc.StructureByteStride = sizeof(uint32_t);

        D3D11_SUBRESOURCE_DATA indexBufferData{};
        indexBufferData.pSysMem = indices.data();
        indexBufferData.SysMemPitch = 0u;
        indexBufferData.SysMemSlicePitch = 0u;

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_IndexBuffer));
    }

    void IndexBuffer::Bind()
    {
        D3D::GetDeviceContext4()->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
    }
}
