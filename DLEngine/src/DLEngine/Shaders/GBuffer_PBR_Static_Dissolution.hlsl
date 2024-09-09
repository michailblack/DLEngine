#include "Include/Buffers.hlsli"
#include "Include/Common.hlsli"
#include "Include/PBR_Resources.hlsli"
#include "Include/Samplers.hlsli"

struct VertexInput
{
    float3 a_Position  : POSITION;
    float3 a_Normal    : NORMAL;
    float3 a_Tangent   : TANGENT;
    float3 a_Bitangent : BITANGENT;
    float2 a_TexCoords : TEXCOORDS;
};

struct TransformInput
{
    float4x4 a_Transform : TRANSFORM;
};

struct InstanceInput
{
    float a_DissolutionDuration : DISSOLUTION_DURATION;
    float a_ElapsedTime         : ELAPSED_TIME;
};

struct VertexOutput
{
    float4                v_Position       : SV_POSITION;
    float3                v_WorldPos       : WORLD_POS;
    float3                v_Normal         : NORMAL;
    float3x3              v_TangentToWorld : TANGENT_TO_WORLD;
    float2                v_TexCoords      : TEXCOORDS;
    nointerpolation float v_Dissolution    : DISSOLUTION_FACTOR;
};

VertexOutput mainVS(VertexInput vsInput, TransformInput transformInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), transformInput.a_Transform);

    vsOutput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);

    vsOutput.v_TexCoords = vsInput.a_TexCoords;

    const float3x3 normalMatrix = ConstructNormalMatrix(transformInput.a_Transform);
    
    vsOutput.v_Normal = mul(vsInput.a_Normal, normalMatrix);
    
    const float3 T = mul(vsInput.a_Tangent, normalMatrix);
    const float3 B = mul(vsInput.a_Bitangent, normalMatrix);
    
    vsOutput.v_TangentToWorld = float3x3(T, B, vsOutput.v_Normal);

    vsOutput.v_Dissolution = saturate(instInput.a_ElapsedTime / instInput.a_DissolutionDuration);
    
    return vsOutput;
}

struct PixelOutput
{
    float4 o_Albedo                 : SV_TARGET0;
    float2 o_MetalnessRoughness     : SV_TARGET1;
    float4 o_GeometrySurfaceNormals : SV_TARGET2;
    float4 o_Emission               : SV_TARGET3;
};

static const float3 EmissionColor = float3(32.0, 32.0, 0.0);

Texture2D<float> t_DissolutionNoiseMap : register(t20);

PixelOutput mainPS(VertexOutput psInput)
{
    const float dissolutionNoise = t_DissolutionNoiseMap.Sample(s_NearestWrap, psInput.v_TexCoords).r;
    
    if (psInput.v_Dissolution < dissolutionNoise)
        discard;
    
    const float3 albedo = t_Albedo.Sample(s_ActiveSampler, psInput.v_TexCoords).rgb;
    
    float metalness = c_DefaultMetalness;
    if (c_HasMetalnessMap)
        metalness = t_Metalness.Sample(s_ActiveSampler, psInput.v_TexCoords).r;

    float roughness = c_DefaultRoughness;
    if (c_HasRoughnessMap)
        roughness = t_Roughness.Sample(s_ActiveSampler, psInput.v_TexCoords).r;

    float3 surfaceNormal = psInput.v_Normal;
    if (c_UseNormalMap)
    {
        float2 normalMapSampleCoords = psInput.v_TexCoords;
        if (c_FlipNormalMapY)
            normalMapSampleCoords.y = 1.0 - normalMapSampleCoords.y;
        const float2 normalBC5 = t_Normal.Sample(s_ActiveSampler, normalMapSampleCoords).rg;
    
        const float3 normalTangentSpace = float3(normalBC5.x, normalBC5.y, sqrt(saturate(1.0 - dot(normalBC5.xy, normalBC5.yx))));
        surfaceNormal = normalize(mul(
            normalTangentSpace,
            psInput.v_TangentToWorld
        ));
    }
    
    PixelOutput psOutput;
    psOutput.o_Albedo = float4(albedo, 1.0);
    psOutput.o_MetalnessRoughness = float2(metalness, roughness);
    psOutput.o_GeometrySurfaceNormals = float4(packOctahedron(normalize(psInput.v_Normal)), packOctahedron(surfaceNormal));
    
    const float falloff = 0.15 / Epsilon;
    const float dissolutionAlpha = saturate((psInput.v_Dissolution - dissolutionNoise) / max(fwidth(psInput.v_Dissolution), Epsilon) / falloff);
    
    psOutput.o_Emission = lerp(float4(EmissionColor, 1.0), float4(0.0, 0.0, 0.0, 1.0), dissolutionAlpha);
    
    return psOutput;
}