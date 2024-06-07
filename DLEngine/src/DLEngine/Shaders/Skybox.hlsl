#include "Global.hlsli"

struct VertexOutput
{
    float4 v_Position  : SV_POSITION;
    float3 v_CornerDir : CORNER_DIR;
};

static const float4 VERTICES_POS[3] = 
{
    float4(-1.0, -1.0, 0, 1.0),
    float4(-1.0, 3.0, 0, 1.0),
    float4(3.0, -1.0, 0, 1.0)
};

VertexOutput mainVS(uint vertexID : SV_VertexID)
{
    VertexOutput vsOutput;
    
    vsOutput.v_Position = VERTICES_POS[vertexID];
    
    float4 vertexWorldPos = mul(vsOutput.v_Position, c_InvViewProjection);
    vertexWorldPos /= vertexWorldPos.w;

    vsOutput.v_CornerDir = normalize(vertexWorldPos.xyz - c_CameraPosition.xyz);
    
    return vsOutput;
}

TextureCube g_Skybox : register(t0);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    return float4(g_Skybox.Sample(g_AnisotropicClamp, psInput.v_CornerDir).xyz, 1.0);
}
