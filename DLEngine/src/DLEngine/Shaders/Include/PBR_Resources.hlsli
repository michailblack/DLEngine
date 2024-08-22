#ifndef _PBR_RESOURCES_HLSLI_
#define _PBR_RESOURCES_HLSLI_

cbuffer PBRMaterial : register(b4)
{
    bool  c_UseNormalMap;
    bool  c_FlipNormalMapY;
    bool  c_HasMetalnessMap;
    float c_DefaultMetalness;
    bool  c_HasRoughnessMap;
    float c_DefaultRoughness;
};

Texture2D<float3> t_Albedo    : register(t9);
Texture2D<float2> t_Normal    : register(t10);
Texture2D<float>  t_Metalness : register(t11);
Texture2D<float>  t_Roughness : register(t12);

TextureCube<float3> t_EnvironmentIrradiance  : register(t13);
TextureCube<float3> t_PrefilteredEnvironment : register(t14);
Texture2D<float3>   t_BRDFLUT                : register(t15);

#endif