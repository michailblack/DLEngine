#pragma once
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/Buffer.h"

#include "DLEngine/Renderer/ShaderInput.h"

namespace DLEngine
{
    enum class VertexBufferUsage
    {
        None = 0,
        Static, Dynamic
    };

    class VertexBuffer
    {
    public:
        virtual ~VertexBuffer() = default;

        virtual Buffer Map() = 0;
        virtual void Unmap() = 0;

        virtual const VertexBufferLayout& GetLayout() const noexcept = 0;
        virtual size_t GetSize() const noexcept = 0;

        static Ref<VertexBuffer> Create(const VertexBufferLayout& layout, const Buffer& buffer, VertexBufferUsage usage = VertexBufferUsage::Static);
        static Ref<VertexBuffer> Create(const VertexBufferLayout& layout, size_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);
    };
}

