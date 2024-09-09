#include "dlpch.h"
#include "IndexBuffer.h"

#include "DLEngine/DirectX/D3D11IndexBuffer.h"

namespace DLEngine
{
    Ref<IndexBuffer> IndexBuffer::Create(const Buffer& buffer)
    {
        return CreateRef<D3D11IndexBuffer>(buffer);
    }
}