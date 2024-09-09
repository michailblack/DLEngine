#pragma once
#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/D3DStates.h"
#include "DLEngine/DirectX/InputLayout.h"
#include "DLEngine/DirectX/Shaders.h"

namespace DLEngine
{
    struct PipelineState
    {
        InputLayout Layout{};
        VertexShader VS{};
        HullShader HS{};
        DomainShader DS{};
        GeometryShader GS{};
        PixelShader PS{};
        RasterizerState Rasterizer{};
        DepthStencilState DepthStencil{};
        D3D_PRIMITIVE_TOPOLOGY Topology{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
    };
}

