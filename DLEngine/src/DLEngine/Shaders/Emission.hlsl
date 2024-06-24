#include "Buffers.hlsli"
#include "Input.hlsli"

struct InstanceInput
{
    float3 a_EmissionLuminance : EMISSION_LUMINANCE;
    uint a_TransformIndex      : TRANSFORM_INDEX;
};

struct VertexOutput
{
    float4 v_Position          : SV_POSITION;
    float3 v_EmissionLuminance : EMISSION_LUMINANCE;
    float3 v_Normal            : NORMAL;
    float3 v_WorldPos          : WORLD_POS;
};

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    float4x4 meshToWorld = mul(c_MeshToModel, c_ModelToWorld[instInput.a_TransformIndex]);
    float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), meshToWorld);

    vsOutput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);

    float3 axisX = normalize(meshToWorld[0].xyz);
    float3 axizY = normalize(meshToWorld[1].xyz);
    float3 axizZ = normalize(meshToWorld[2].xyz);
    vsOutput.v_Normal = vsInput.a_Normal.x * axisX + vsInput.a_Normal.y * axizY + vsInput.a_Normal.z * axizZ;
    vsOutput.v_EmissionLuminance = instInput.a_EmissionLuminance;

    return vsOutput;
}

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    float3 normal = normalize(psInput.v_Normal);
    float3 viewDir = normalize(c_CameraPosition.xyz - psInput.v_WorldPos);

    float3 normedEmission = psInput.v_EmissionLuminance / max(psInput.v_EmissionLuminance.x, max(psInput.v_EmissionLuminance.y, max(psInput.v_EmissionLuminance.z, 1.0)));

    float NoV = dot(normal, viewDir);

    return float4(lerp(normedEmission * 0.33, psInput.v_EmissionLuminance, pow(max(0.0, NoV), 8.0)), 1.0);
}
