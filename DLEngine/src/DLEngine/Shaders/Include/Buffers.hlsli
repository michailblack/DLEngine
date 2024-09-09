#ifndef _BUFFERS_HLSLI_
#define _BUFFERS_HLSLI_

cbuffer Camera : register(b0)
{
    float4x4 c_Projection;
    float4x4 c_InvProjection;
    float4x4 c_View;
    float4x4 c_InvView;
    float4x4 c_ViewProjection;
    float4x4 c_InvViewProjection;
    float3   c_CameraPosition;
    float3   c_BL;
    float3   c_BL2TL;
    float3   c_BL2BR;
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

cbuffer ShadowMapping : register(b2)
{
    uint  c_ShadowMapSize;
    float c_ShadowBias;
    bool  c_UseDirectionalShadows;
    bool  c_UseOmnidirectionalShadows;
    bool  c_UseSpotShadows;
    bool  c_UsePCF;
};

cbuffer LightsCount : register(b3)
{
    uint c_DirectionalLightsCount;
    uint c_PointLightsCount;
    uint c_SpotLightsCount;
};

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

StructuredBuffer<float4x4> t_DirectionalLightsPOVs : register(t3);
StructuredBuffer<float4x4> t_PointLightsPOVs       : register(t4); // x6 matrices for each cube face per light
StructuredBuffer<float4x4> t_SpotLightsPOVs        : register(t5);

Texture2DArray<float>   t_DirectionalShadowMaps     : register(t6);
TextureCubeArray<float> t_OmnidirectionalShadowMaps : register(t7);
Texture2DArray<float>   t_SpotShadowMaps            : register(t8);

#endif