#include "dlpch.h"
#include "StructuredBuffer.h"

#include "DLEngine/DirectX/D3D11StructuredBuffer.h"

namespace DLEngine
{
    Ref<StructuredBuffer> StructuredBuffer::Create(size_t structureSize, uint32_t elementsCount)
    {
        return CreateRef<D3D11StructuredBuffer>(structureSize, elementsCount);
    }
}