#include "dlpch.h"
#include "Pipeline.h"

#include "DLEngine/DirectX/D3D11Pipeline.h"

namespace DLEngine
{
    Ref<Pipeline> Pipeline::Create(const PipelineSpecification& specification)
    {
        return CreateRef<D3D11Pipeline>(specification);
    }
}
