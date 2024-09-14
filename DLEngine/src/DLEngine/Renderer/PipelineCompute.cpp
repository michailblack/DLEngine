#include "dlpch.h"
#include "PipelineCompute.h"

#include "DLEngine/DirectX/D3D11PipelineCompute.h"

namespace DLEngine
{
    Ref<PipelineCompute> PipelineCompute::Create(const PipelineComputeSpecification& specificaton)
    {
        return CreateRef<D3D11PipelineCompute>(specificaton);
    }

}