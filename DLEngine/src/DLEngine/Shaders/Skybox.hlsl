#include "Global.hlsli"

struct VertexOutput
{
    float4 v_Position  : SV_POSITION;
};

VertexOutput mainVS(uint vertexID : SV_VertexID)
{
    VertexOutput vsOutput;
    
    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    vsOutput.v_Position = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    
    return vsOutput;
}

TextureCube<float3> g_Skybox : register(t0);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    float2 pixelXY = psInput.v_Position.xy / c_Resolution;
    float3 sampleVec = c_BL.xyz + pixelXY.x * c_BL2BR.xyz + (1.0 - pixelXY.y) * c_BL2TL.xyz;

    return float4(g_Skybox.Sample(g_ActiveSampler, sampleVec).rgb, 1.0);
}
