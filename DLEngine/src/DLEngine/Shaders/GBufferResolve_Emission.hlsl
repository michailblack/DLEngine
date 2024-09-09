#include "Include/GBufferResources.hlsli"
#include "Include/Samplers.hlsli"

struct VertexOutput
{
    float4 v_Position : SV_POSITION;
    float2 v_TexCoords : TEXCOORDS;
};

VertexOutput mainVS(uint vertexID : SV_VertexID)
{
    VertexOutput vsOutput;
    
    vsOutput.v_TexCoords = float2((vertexID << 1) & 2, vertexID & 2);
    vsOutput.v_Position = float4(vsOutput.v_TexCoords * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    
    return vsOutput;
}

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    const float3 emission = t_GBufferEmission.Sample(s_TrilinearClamp, psInput.v_TexCoords).rgb;
    return float4(emission, 1.0);
}