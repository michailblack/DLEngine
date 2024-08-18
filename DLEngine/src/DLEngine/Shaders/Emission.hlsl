#include "Include/Buffers.hlsli"

struct VertexInput
{
    float3 a_Position  : POSITION;
    float3 a_Normal    : NORMAL;
    float3 a_Tangent   : TANGENT;
    float3 a_Bitangent : BITANGENT;  
    float2 a_TexCoords : TEXCOORDS;
};

struct InstanceInput
{
    float4x4 a_Transform        : TRANSFORM;
    float3   a_EmissionRadiance : RADIANCE;
};

struct VertexOutput
{
    float4 v_Position         : SV_POSITION;
    float3 v_EmissionRadiance : RADIANCE;
    float3 v_Normal           : NORMAL;
    float3 v_WorldPos         : WORLD_POS;
};

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), instInput.a_Transform);

    vsOutput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);

    float3 axisX = normalize(instInput.a_Transform[0].xyz);
    float3 axizY = normalize(instInput.a_Transform[1].xyz);
    float3 axizZ = normalize(instInput.a_Transform[2].xyz);
    vsOutput.v_Normal = vsInput.a_Normal.x * axisX + vsInput.a_Normal.y * axizY + vsInput.a_Normal.z * axizZ;
    vsOutput.v_EmissionRadiance = instInput.a_EmissionRadiance;

    return vsOutput;
}

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    float3 normal = normalize(psInput.v_Normal);
    float3 viewDir = normalize(c_CameraPosition - psInput.v_WorldPos);

    float3 normedEmission = psInput.v_EmissionRadiance / max(psInput.v_EmissionRadiance.x, max(psInput.v_EmissionRadiance.y, max(psInput.v_EmissionRadiance.z, 1.0)));

    float NoV = dot(normal, viewDir);

    return float4(lerp(normedEmission * 0.33, psInput.v_EmissionRadiance, pow(max(0.0, NoV), 8.0)), 1.0);
}
