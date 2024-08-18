cbuffer Camera : register(b0)
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

cbuffer PBRSettings : register(b1)
{
    float3 c_IndirectLightingRadiance;
    float  c_OverwrittenRoughness;
    bool   c_OverwriteRoughness;
    bool   c_UseIBL;
    bool   c_UseDiffuseReflectoins;
    bool   c_UseSpecularReflections;
    
}

struct DirectionalLight
{
    float3 Direction;
    float3 Radiance;
    float SolidAngle;
};

StructuredBuffer<DirectionalLight> t_DirectionalLights : register(t0);

struct PointLight
{
    float3 Position;
    float3 Radiance;
    float Radius;
};

StructuredBuffer<PointLight> t_PointLights : register(t1);

struct SpotLight
{
    float3 Position;
    float3 Direction;
    float3 Radiance;
    float Radius;
    float InnerCutoffCos;
    float OuterCutoffCos;
};

StructuredBuffer<SpotLight> t_SpotLights : register(t2);

StructuredBuffer<float4x4> t_LightsViewProjection : register(t3);