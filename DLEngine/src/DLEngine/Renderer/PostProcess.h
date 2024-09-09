#pragma once
#include "DLEngine/DirectX/ConstantBuffer.h"
#include "DLEngine/DirectX/PipelineState.h"

namespace DLEngine
{
    class Texture2D;

    class PostProcess
    {
    public:
        void Create();

        void Resolve(const Texture2D& src, const Texture2D& dst) const;

    private:
        PipelineState m_PipelineState;
    };
}
