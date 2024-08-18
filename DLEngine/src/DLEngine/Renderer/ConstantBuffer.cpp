#include "dlpch.h"
#include "ConstantBuffer.h"

#include "DLEngine/DirectX/D3D11ConstantBuffer.h"

namespace DLEngine
{
    Ref<ConstantBuffer> ConstantBuffer::Create(size_t size)
    {
        return CreateRef<D3D11ConstantBuffer>(size);
    }
}