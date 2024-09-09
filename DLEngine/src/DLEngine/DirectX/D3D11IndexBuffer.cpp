#include "dlpch.h"
#include "D3D11IndexBuffer.h"

#include "DLEngine/DirectX/D3D11Context.h"

namespace DLEngine
{
    D3D11IndexBuffer::D3D11IndexBuffer(const Buffer& buffer)
    {
        D3D11_BUFFER_DESC indexBufferDesc{};
        indexBufferDesc.ByteWidth = static_cast<UINT>(buffer.Size);
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0u;
        indexBufferDesc.MiscFlags = 0u;
        indexBufferDesc.StructureByteStride = sizeof(uint32_t);

        D3D11_SUBRESOURCE_DATA indexBufferData{};
        indexBufferData.pSysMem = buffer.Data;
        indexBufferData.SysMemPitch = 0u;
        indexBufferData.SysMemSlicePitch = 0u;

        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_D3D11IndexBuffer));
    }
}