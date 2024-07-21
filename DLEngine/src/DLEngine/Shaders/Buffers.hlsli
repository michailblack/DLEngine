cbuffer PerFrame : register(b0)
{
    float2 c_Resolution;
    float2 c_MousePos;
    float c_TimeMS;
    float c_TimeS;
};

cbuffer PerView : register(b1)
{
    float4x4 c_Projection;
    float4x4 c_InvProjection;
    float4x4 c_View;
    float4x4 c_InvView;
    float4x4 c_ViewProjection;
    float4x4 c_InvViewProjection;
    float3 c_CameraPosition;
    float3 c_BL;
    float3 c_BL2TL;
    float3 c_BL2BR;
};

cbuffer MeshInstance : register(b2)
{
    float4x4 c_MeshToModel;
    float4x4 c_ModelToMesh;
};

StructuredBuffer<float4x4> t_ModelToWorld : register(t0);
StructuredBuffer<float4x4> t_WorldToModel : register(t1);

#include "LightCasters.hlsli"

StructuredBuffer<DirectionalLight> t_DirectionalLights : register(t2);
StructuredBuffer<PointLight>       t_PointLights       : register(t3);
StructuredBuffer<SpotLight>        t_SpotLights        : register(t4);
