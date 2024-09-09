#pragma once
#include "DLEngine/Core/Base.h"

namespace DLEngine
{
    class RendererContext
    {
    public:
        virtual ~RendererContext() = default;

        virtual void Init() = 0;

        static Ref<RendererContext> Create();
    };
}

