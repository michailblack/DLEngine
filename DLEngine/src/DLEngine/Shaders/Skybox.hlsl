#include "Include/Buffers.hlsli"
#include "Include/Samplers.hlsli"

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
    vsOutput.v_CubemapSample = c_BL + uv.x * c_BL2BR + (1.0 - uv.y) * c_BL2TL;
    
    return vsOutput;
}

TextureCube<float3> t_Skybox : register(t0);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    return float4(t_Skybox.Sample(s_Anisotropic8Clamp, psInput.v_CubemapSample).rgb, 1.0);
}
