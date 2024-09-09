#include "Include/Buffers.hlsli"
#include "Include/Samplers.hlsli"

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
    float4 v_Position  : SV_POSITION;
    float2 v_TexCoords : TEXCOORDS;
};

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float4 worldPos = mul(float4(vsInput.a_Position, 1.0), instInput.a_Transform);

    vsOutput.v_Position = mul(worldPos, c_ViewProjection);
    vsOutput.v_TexCoords = vsInput.a_TexCoords;

    return vsOutput;
}

Texture2D<float3> t_Texture : register(t0);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    float3 color = t_Texture.Sample(s_ActiveSampler, psInput.v_TexCoords).xyz;
    return float4(color, 1.0);
}
