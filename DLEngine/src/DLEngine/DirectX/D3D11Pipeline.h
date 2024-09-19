#pragma once
#include "DLEngine/Renderer/Pipeline.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class D3D11Pipeline : public Pipeline
    {
    public:
        D3D11Pipeline(const PipelineSpecification& specificaton);

        const PipelineSpecification& GetSpecification() const noexcept override { return m_Specification; }

    private:
        PipelineSpecification m_Specification;
    };
}