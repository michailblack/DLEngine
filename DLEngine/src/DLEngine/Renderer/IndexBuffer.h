#pragma once
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/Buffer.h"

namespace DLEngine
{
    class IndexBuffer
    {
    public:
        virtual ~IndexBuffer() = default;

        static Ref<IndexBuffer> Create(const Buffer& buffer);
    };
}

