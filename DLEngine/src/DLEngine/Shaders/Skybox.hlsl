#include "Global.hlsli"

struct VertexOutput
{
    float4 v_Position  : SV_POSITION;
    float3 v_CornerDir : CORNER_DIR;
};

VertexOutput mainVS(uint vertexID : SV_VertexID)
{
    VertexOutput vsOutput;
    
    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    vsOutput.v_Position = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);

    float3 top = lerp(c_FrustumTopLeftDir.xyz, c_FrustumTopRightDir.xyz, float3(uv.x, uv.x, uv.x));
    float3 bottom = lerp(c_FrustumBottomLeftDir.xyz, c_FrustumBottomRightDir.xyz, float3(uv.x, uv.x, uv.x));
    
    vsOutput.v_CornerDir = lerp(top, bottom, float3(uv.y, uv.y, uv.y));
    
    return vsOutput;
}

TextureCube<float3> g_Skybox : register(t0);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    return float4(g_Skybox.Sample(g_ActiveSampler, psInput.v_CornerDir).rgb, 1.0);
}
