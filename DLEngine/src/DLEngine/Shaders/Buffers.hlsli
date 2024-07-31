cbuffer PerFrame : register(b0)
{
    float c_TimeMS;
    float c_TimeS;
    float2 c_Resolution;
    float2 c_MousePos;
};

cbuffer PerView : register(b1)
{
    float4x4 c_Projection;
    float4x4 c_InvProjection;
    float4x4 c_View;
    float4x4 c_InvView;
    float4x4 c_ViewProjection;
    float4x4 c_InvViewProjection;
    float4 c_CameraPosition;
    float4 c_BL;
    float4 c_BL2TL;
    float4 c_BL2BR;
};

static const uint MAX_CONSTANT_BUFFER_FLOAT4_ELEMENTS_COUNT = 4096;

cbuffer Transforms : register(b2)
{
    float4x4 c_ModelToWorld[MAX_CONSTANT_BUFFER_FLOAT4_ELEMENTS_COUNT / 4];
};

cbuffer InvTransforms : register(b3)
{
    float4x4 c_WorldToModel[MAX_CONSTANT_BUFFER_FLOAT4_ELEMENTS_COUNT / 4];
};

cbuffer MeshInstance : register(b4)
{
    float4x4 c_MeshToModel;
    float4x4 c_ModelToMesh;
};

#include "LightCasters.hlsli"

cbuffer DirectionalLights : register(b5)
{
    DirectionalLight c_DirectionalLights[MAX_CONSTANT_BUFFER_FLOAT4_ELEMENTS_COUNT / 8];
};

cbuffer PointLights : register(b6)
{
    PointLight c_PointLights[MAX_CONSTANT_BUFFER_FLOAT4_ELEMENTS_COUNT / 12];
};

cbuffer SpotLights : register(b7)
{
    SpotLight c_SpotLights[MAX_CONSTANT_BUFFER_FLOAT4_ELEMENTS_COUNT / 16];
};

cbuffer LightsCount : register(b8)
{
    uint c_DirectionalLightsCount;
    uint c_PointLightsCount;
    uint c_SpotLightsCount;
    float _padding;
};
