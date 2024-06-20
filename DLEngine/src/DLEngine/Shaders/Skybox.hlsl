#include "Buffers.hlsli"
#include "Samplers.hlsli"

struct VertexOutput
{
    float4 v_Position      : SV_POSITION;
    float3 v_CubemapSample : CUBEMAP_SAMPLE;
};

VertexOutput mainVS(uint vertexID : SV_VertexID)
{
    VertexOutput vsOutput;
    
    const float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    vsOutput.v_Position = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    vsOutput.v_CubemapSample = c_BL.xyz + uv.x * c_BL2BR.xyz + (1.0 - uv.y) * c_BL2TL.xyz;
    
    return vsOutput;
}

TextureCube<float3> g_Skybox : register(t0);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    return float4(g_Skybox.Sample(g_ActiveSampler, psInput.v_CubemapSample).rgb, 1.0);
}
