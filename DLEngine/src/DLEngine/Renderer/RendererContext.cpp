#include "dlpch.h"
#include "RendererContext.h"

#include "DLEngine/DirectX/D3D11Context.h"

namespace DLEngine
{
    Ref<RendererContext> RendererContext::Create()
    {
        return CreateRef<D3D11Context>();
    }
}