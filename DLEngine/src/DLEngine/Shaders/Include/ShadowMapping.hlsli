#ifndef _SHADOWMAPPING_HLSLI_
#define _SHADOWMAPPING_HLSLI_

#include "Buffers.hlsli"
#include "Common.hlsli"
#include "Samplers.hlsli"

static const float2 OrthoOffsets[4] =
{
    float2(-1.5,  0.5),
    float2( 0.5,  0.5),
    float2(-1.5, -1.5),
    float2( 0.5, -1.5),
};

static const float Sqrt2 = sqrt(2.0);
static const float InvSqrt2 = 1.0 / Sqrt2;

//const float3 offset = c_ShadowMapWorldTexelSize / Sqrt2 * (geometryNormal - 0.9 * c_LightDirection * dot(geometryNormal, c_LightDirection));

float Ortho_PCF(Texture2DArray<float> shadowMaps, float3 sampleUVW, float compareValue, float texelSize)
{
    const float2 offset = frac(sampleUVW.xy > float2(0.25, 0.25)) ? float2(1.0, 1.0) : float2(0.0, 0.0);
    
    float visibility = 0.0;
    [unroll(4)]
    for (uint i = 0; i < 4; ++i)
    {
        const float2 sampleOffset = OrthoOffsets[i] + offset;
        visibility += shadowMaps.SampleCmpLevelZero(s_BilinearBorderCmp, sampleUVW + float3(sampleOffset * float2(texelSize, texelSize), 0.0), compareValue).x;
    }
    visibility *= 0.25;
    
    if (!c_UsePCF)
        visibility = shadowMaps.SampleCmpLevelZero(s_BilinearBorderCmp, sampleUVW, compareValue).x;

    return smoothstep(0.33, 1.0, visibility);
}

float VisibilityForDirectionalLight(uint directionalLightIndex, float3 worldPos, float3 geometryNormal)
{
    const float3 lightToFragmentDir = t_DirectionalLights[directionalLightIndex].Direction;
    const float4x4 lightPOV = t_DirectionalLightsPOVs[directionalLightIndex];
    
    const float texelSize = 1.0 / c_ShadowMapSize;
    
    const float3 depthOffset = -lightToFragmentDir * c_ShadowBias;
    const float3 normalOffset = geometryNormal * texelSize;
    
    const float3 lightPOVPos = mul(float4(worldPos + depthOffset, 1.0), lightPOV).xyz;
    const float2 lightPOVUV = lightPOVPos.xy * 0.5 + 0.5;
    const float3 sampleUVW = float3(lightPOVUV.x, 1.0 - lightPOVUV.y, directionalLightIndex);
    
    return Ortho_PCF(t_DirectionalShadowMaps, sampleUVW, lightPOVPos.z, texelSize);
}

float VisibilityForPointLight(uint pointLightIndex, float3 worldPos, float3 geometryNormal)
{
    const float3 pointLightPos = t_PointLights[pointLightIndex].Position;
    const float3 lightToFragment = worldPos - pointLightPos;
    const float3 lightToFragmentDir = normalize(lightToFragment);
    const uint cubeFaceIndex = CubeFaceIndex(lightToFragmentDir);
    
    const float4x4 lightPOV = t_PointLightsPOVs[pointLightIndex * 6u + cubeFaceIndex];
    
    const float3 depthOffset = -lightToFragmentDir * c_ShadowBias;

    const float4 lightPOVPos = mul(float4(worldPos + depthOffset, 1.0), lightPOV);
    const float compareDepth = lightPOVPos.z / lightPOVPos.w;
    const float linearDepth = lightPOVPos.w;
    
    const float texelSize = linearDepth * 2.0 / float(c_ShadowMapSize);
    const float3 normalOffset = geometryNormal * texelSize;
    
    const float4 sampleVec = float4(normalize(lightToFragment + normalOffset), pointLightIndex);
    
    return t_OmnidirectionalShadowMaps.SampleCmpLevelZero(s_BilinearBorderCmp, sampleVec, compareDepth).x;
}

float VisibilityForSpotLight(uint spotLightIndex, float3 worldPos, float3 geometryNormal)
{
    const float3 spotLightPos = t_SpotLights[spotLightIndex].Position;
    const float3 lightToFragment = worldPos - spotLightPos;
    const float3 lightToFragmentDir = normalize(lightToFragment);
    
    const float4x4 lightPOV = t_SpotLightsPOVs[spotLightIndex];
    
    const float3 depthOffset = -lightToFragmentDir * c_ShadowBias;
    
    float4 lightPOVPos = mul(float4(worldPos + depthOffset, 1.0), lightPOV);
    const float linearDepth = lightPOVPos.w;
    
    const float texelSize = linearDepth * 2.0 / float(c_ShadowMapSize);
    const float3 normalOffset = geometryNormal * texelSize;
    
    lightPOVPos = mul(float4(worldPos + depthOffset + normalOffset, 1.0), lightPOV);
    const float3 projectedLightPOVPos = lightPOVPos.xyz / lightPOVPos.w;
    
    const float2 lightPOVUV = projectedLightPOVPos.xy * 0.5 + 0.5;
    const float3 sampleUVW = float3(lightPOVUV.x, 1.0 - lightPOVUV.y, spotLightIndex);
    
    return t_SpotShadowMaps.SampleCmpLevelZero(s_BilinearBorderCmp, sampleUVW, projectedLightPOVPos.z).x;
}

#endif