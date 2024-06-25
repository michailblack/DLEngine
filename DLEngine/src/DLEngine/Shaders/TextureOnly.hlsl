#include "Global.hlsli"

cbuffer MeshInstance : register(b2)
{
    float4x4 c_MeshToModel;
    float4x4 c_ModelToMesh;
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
    float4x4 a_ModelToWorld : TRANSFORM;
    float _emptyInstance    : _empty;
};

struct VertexOutput
{
    float4 v_Position  : SV_POSITION;
    float2 v_TexCoords : TEXCOORDS;
};

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    float4 worldPos = mul(float4(vsInput.a_Position, 1.0), c_MeshToModel);
    worldPos = mul(worldPos, instInput.a_ModelToWorld);

    vsOutput.v_Position = mul(worldPos, c_ViewProjection);
    vsOutput.v_TexCoords = vsInput.a_TexCoords;

    return vsOutput;
}

Texture2D<float3> g_Texture : register(t0);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    float3 color = g_Texture.Sample(g_ActiveSampler, psInput.v_TexCoords).xyz;
    return float4(color, 1.0);
}
