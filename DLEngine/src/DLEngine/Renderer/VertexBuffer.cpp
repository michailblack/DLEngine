#include "dlpch.h"
#include "VertexBuffer.h"

#include "DLEngine/DirectX/D3D11VertexBuffer.h"

#include "DLEngine/Renderer/Shader.h"

namespace DLEngine
{
    Ref<VertexBuffer> VertexBuffer::Create(const VertexBufferLayout& layout, const Buffer& buffer, VertexBufferUsage usage)
    {
        return CreateRef<D3D11VertexBuffer>(layout, buffer, usage);
    }

    Ref<VertexBuffer> VertexBuffer::Create(const VertexBufferLayout& layout, size_t size, VertexBufferUsage usage)
    {
        return CreateRef<D3D11VertexBuffer>(layout, size, usage);
    }
}