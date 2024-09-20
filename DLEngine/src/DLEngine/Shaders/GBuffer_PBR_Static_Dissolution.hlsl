#include "Include/Lighting.hlsli"

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
    uint2 a_InstanceUUID        : INSTANCE_UUID;
    float a_DissolutionDuration : DISSOLUTION_DURATION;
    float a_ElapsedTime         : ELAPSED_TIME;
};

struct VertexOutput
{
    float4                v_Position          : SV_POSITION;
    float3                v_WorldPos          : WORLD_POS;
    float3                v_Normal            : NORMAL;
    float3x3              v_TangentToWorld    : TANGENT_TO_WORLD;
    float2                v_TexCoords         : TEXCOORDS;
    nointerpolation uint2 v_InstanceUUID      : INSTANCE_UUID;
    nointerpolation float v_DissolutionFactor : DISSOLUTION_FACTOR;
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

    vsOutput.v_DissolutionFactor = saturate(instInput.a_ElapsedTime / instInput.a_DissolutionDuration);
    
    vsOutput.v_InstanceUUID = instInput.a_InstanceUUID;
    
    return vsOutput;
}

struct PixelOutput
{
    float4 o_Albedo                 : SV_TARGET0;
    float2 o_MetalnessRoughness     : SV_TARGET1;
    float4 o_GeometrySurfaceNormals : SV_TARGET2;
    float4 o_Emission               : SV_TARGET3;
    uint2  o_InstanceUUID           : SV_TARGET4;
};

static const float3 EmissionColor = float3(32.0, 32.0, 0.0);

Texture2D<float> t_DissolutionNoiseMap : register(t20);

PixelOutput mainPS(VertexOutput psInput)
{
    const float dissolutionNoise = t_DissolutionNoiseMap.Sample(s_NearestWrap, psInput.v_TexCoords).r;
    
    const float dissolutionEpsilon = 0.15; // To prevent dissolution dash when dissolution noise is 1.0
    const float fragmentDissolution = psInput.v_DissolutionFactor + dissolutionEpsilon;
    
    clip(fragmentDissolution - dissolutionNoise);
    
    const Surface surface = CalculatePBR_Surface(psInput.v_TexCoords, psInput.v_Normal, psInput.v_TangentToWorld);
    
    PixelOutput psOutput;
    psOutput.o_Albedo = float4(surface.Albedo, 1.0);
    psOutput.o_MetalnessRoughness = float2(surface.Metalness, surface.Roughness);
    psOutput.o_GeometrySurfaceNormals = float4(packOctahedron(surface.GeometryNormal), packOctahedron(surface.SurfaceNormal));
    
    const float falloff = dissolutionEpsilon / Epsilon;
    const float dissolutionAlpha = saturate((fragmentDissolution - dissolutionNoise) / max(fwidth(fragmentDissolution), Epsilon) / falloff);
    psOutput.o_Emission = lerp(float4(EmissionColor, 1.0), float4(0.0, 0.0, 0.0, 1.0), dissolutionAlpha);
    
    psOutput.o_InstanceUUID = psInput.v_InstanceUUID;
    
    return psOutput;
}