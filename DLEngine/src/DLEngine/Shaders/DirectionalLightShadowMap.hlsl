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

struct InstanceInput
{
    float4x4 a_Transform : TRANSFORM;
};

struct VertexOutput
{
    float4 v_Position : SV_POSITION;
};

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float3 vertexPos = mul(float4(vsInput.a_Position, 1.0), instInput.a_Transform).xyz;
    
    vsOutput.v_Position = mul(float4(vertexPos, 1.0), c_ViewProjection);

    return vsOutput;
}