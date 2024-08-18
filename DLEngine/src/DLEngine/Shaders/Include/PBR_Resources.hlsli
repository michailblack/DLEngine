cbuffer PBRMaterial : register(b2)
{
    bool  c_UseNormalMap;
    bool  c_FlipNormalMapY;
    bool  c_HasMetalnessMap;
    float c_DefaultMetalness;
    bool  c_HasRoughnessMap;
    float c_DefaultRoughness;
};

Texture2D<float3> t_Albedo    : register(t4);
Texture2D<float2> t_Normal    : register(t5);
Texture2D<float>  t_Metalness : register(t6);
Texture2D<float>  t_Roughness : register(t7);

TextureCube<float3> t_EnvironmentIrradiance  : register(t8);
TextureCube<float3> t_PrefilteredEnvironment : register(t9);
Texture2D<float3>   t_BRDFLUT                : register(t10);

Texture2DArray<float>   t_DirectionalShadowMaps     : register(t11);
TextureCubeArray<float> t_OmnidirectionalShadowMaps : register(t12);