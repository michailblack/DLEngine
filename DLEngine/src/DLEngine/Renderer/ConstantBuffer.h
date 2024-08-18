#pragma once
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/Buffer.h"

namespace DLEngine
{
    class ConstantBuffer
    {
    public:
        virtual ~ConstantBuffer() = default;

        virtual void SetData(const Buffer& buffer) = 0;
        
        virtual const Buffer& GetLocalData() const noexcept = 0;

        static Ref<ConstantBuffer> Create(size_t size);
    };
}

