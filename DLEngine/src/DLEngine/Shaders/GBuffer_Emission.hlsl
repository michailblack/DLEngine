#include "Include/Buffers.hlsli"
#include "Include/Common.hlsli"

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
    float3 a_EmissionRadiance : RADIANCE;
    uint2 a_InstanceUUID      : INSTANCE_UUID;
};

struct VertexOutput
{
    float4                 v_Position         : SV_POSITION;
    nointerpolation float3 v_EmissionRadiance : RADIANCE;
    float3                 v_Normal           : NORMAL;
    float3                 v_WorldPos         : WORLD_POS;
    nointerpolation uint2  v_InstanceUUID     : INSTANCE_UUID;
};

VertexOutput mainVS(VertexInput vsInput, TransformInput transformInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), transformInput.a_Transform);

    vsOutput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);
    
    const float3x3 normalMatrix = ConstructNormalMatrix(transformInput.a_Transform);
    vsOutput.v_Normal = mul(vsInput.a_Normal, normalMatrix);
    
    vsOutput.v_EmissionRadiance = instInput.a_EmissionRadiance;
    vsOutput.v_InstanceUUID = instInput.a_InstanceUUID;

    return vsOutput;
}

struct PixelOutput
{
    float4 o_Emission     : SV_TARGET3;
    uint2  o_InstanceUUID : SV_TARGET4;
};

PixelOutput mainPS(VertexOutput psInput)
{
    PixelOutput psOutput;
    
    float3 normal = normalize(psInput.v_Normal);
    float3 viewDir = normalize(c_CameraPosition - psInput.v_WorldPos);

    float3 normedEmission = psInput.v_EmissionRadiance / max(psInput.v_EmissionRadiance.x, max(psInput.v_EmissionRadiance.y, max(psInput.v_EmissionRadiance.z, 1.0)));

    float NoV = dot(normal, viewDir);
    
    psOutput.o_Emission = float4(lerp(normedEmission * 0.33, psInput.v_EmissionRadiance, pow(max(0.0, NoV), 8.0)), 1.0);
    psOutput.o_InstanceUUID = psInput.v_InstanceUUID;

    return psOutput;
}