#ifndef _SHADOWMAPPING_HLSLI_
#define _SHADOWMAPPING_HLSLI_

#include "Include/Buffers.hlsli"
#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"

static const float2 OrthoOffsets[4] =
{
    float2(-1.5,  0.5),
    float2( 0.5,  0.5),
    float2(-1.5, -1.5),
    float2( 0.5, -1.5),
};

float Ortho_PCF(Texture2DArray<float> shadowMaps, float3 sampleUVW, float compareValue)
{
    const float2 texelSize = float2(1.0 / c_ShadowMapSize, 1.0 / c_ShadowMapSize);
    const float2 offset = frac(sampleUVW.xy > float2(0.25, 0.25)) ? float2(1.0, 1.0) : float2(0.0, 0.0);
    
    float visibility = 0.0;
    [unroll(4)]
    for (uint i = 0; i < 4; ++i)
    {
        const float2 sampleOffset = OrthoOffsets[i] + offset;
        visibility += shadowMaps.SampleCmpLevelZero(s_BilinearBorderCmp, sampleUVW + float3(sampleOffset * texelSize, 0.0), compareValue).x;
    }
    visibility *= 0.25;
    
    if (!c_UsePCF)
        visibility = shadowMaps.SampleCmpLevelZero(s_BilinearBorderCmp, sampleUVW, compareValue).x;

    return visibility;
}

float VisibilityForDirectionalLight(uint directionalLightIndex, float3 worldPos)
{
    const float3 lightPOVPos = mul(float4(worldPos, 1.0), t_DirectionalLightsPOVs[directionalLightIndex]).xyz;
    const float2 lightPOVUV = lightPOVPos.xy * 0.5 + 0.5;
    const float3 sampleUVW = float3(lightPOVUV.x, 1.0 - lightPOVUV.y, directionalLightIndex);
    
    return Ortho_PCF(t_DirectionalShadowMaps, sampleUVW, lightPOVPos.z);
}

float VisibilityForPointLight(uint pointLightIndex, float3 worldPos)
{
    const float3 pointLightPos = t_PointLights[pointLightIndex].Position;
    const float3 lightDir = normalize(worldPos - pointLightPos);
    const uint cubeFaceIndex = CubeFaceIndex(lightDir);
    
    const float4x4 lightPOV = t_PointLightsPOVs[pointLightIndex * 6u + cubeFaceIndex];

    const float4 lightPOVPos = mul(float4(worldPos, 1.0), lightPOV);
    const float3 projectedLightPOVPos = lightPOVPos.xyz / lightPOVPos.w;
    
    const float4 sampleVec = float4(lightDir, pointLightIndex);
    
    return t_OmnidirectionalShadowMaps.SampleCmpLevelZero(s_BilinearBorderCmp, sampleVec, projectedLightPOVPos.z).x;
}

float VisibilityForSpotLight(uint spotLightIndex, float3 worldPos)
{
    const float3 spotLightPos = t_SpotLights[spotLightIndex].Position;
    const float3 lightDir = normalize(worldPos - spotLightPos);
    
    const float4 lightPOVPos = mul(float4(worldPos, 1.0), t_SpotLightsPOVs[spotLightIndex]);
    const float3 projectedLightPOVPos = lightPOVPos.xyz / lightPOVPos.w;
    
    const float2 lightPOVUV = projectedLightPOVPos.xy * 0.5 + 0.5;
    const float3 sampleUVW = float3(lightPOVUV.x, 1.0 - lightPOVUV.y, spotLightIndex);
    
    return t_SpotShadowMaps.SampleCmpLevelZero(s_BilinearBorderCmp, sampleUVW, projectedLightPOVPos.z).x;
}

#endif