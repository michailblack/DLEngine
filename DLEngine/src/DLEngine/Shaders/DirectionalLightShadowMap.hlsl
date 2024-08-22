#include "Include/Buffers.hlsli"

cbuffer DirectionalLightShadowData : register(b4)
{
    float3 c_LightDirection;
    float  c_ShadowMapWorldTexelSize;
};

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
    float4x4 a_Transform : TRANSFORM;
};

struct VertexOutput
{
    float4 v_Position : SV_POSITION;
};

static const float Sqrt2 = sqrt(2);

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float3 vertexPos = mul(float4(vsInput.a_Position, 1.0), instInput.a_Transform).xyz;

    float3 axisX = normalize(instInput.a_Transform[0].xyz);
    float3 axizY = normalize(instInput.a_Transform[1].xyz);
    float3 axizZ = normalize(instInput.a_Transform[2].xyz);
    const float3 geometryNormal = normalize(vsInput.a_Normal.x * axisX + vsInput.a_Normal.y * axizY + vsInput.a_Normal.z * axizZ);

    const float3 offset = c_ShadowMapWorldTexelSize / Sqrt2 * (geometryNormal - 0.9 * c_LightDirection * dot(geometryNormal, c_LightDirection));

    vsOutput.v_Position = mul(float4(vertexPos - offset, 1.0), c_ViewProjection);

    return vsOutput;
}