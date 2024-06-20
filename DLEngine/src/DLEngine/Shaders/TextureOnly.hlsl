#include "Buffers.hlsli"
#include "Input.hlsli"
#include "Samplers.hlsli"

struct InstanceInput
{
    float _emptyInstance   : _empty;
    float a_TransformIndex : TRANSFORM_INDEX;
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
    worldPos = mul(worldPos, c_ModelToWorld[uint(instInput.a_TransformIndex)]);

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
