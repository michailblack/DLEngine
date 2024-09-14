#include "dlpch.h"
#include "StructuredBuffer.h"

#include "DLEngine/DirectX/D3D11StructuredBuffer.h"

namespace DLEngine
{
    Ref<StructuredBuffer> StructuredBuffer::Create(size_t structureSize, uint32_t elementsCount, BufferViewType viewType)
    {
        return CreateRef<D3D11StructuredBuffer>(structureSize, elementsCount, viewType);
    }

    Ref<PrimitiveBuffer> PrimitiveBuffer::Create(uint32_t elementsCount, BufferViewType viewType, uint32_t bufferMiscFlags)
    {
        return CreateRef<D3D11PrimitiveBuffer>(elementsCount, viewType, bufferMiscFlags);
    }

}