#include "Samplers.hlsli"

struct VertexOutput
{
    float4 v_Position  : SV_POSITION;
    float2 v_TexCoords : TEXCOORDS;
};

static const float VERTICES_XY[] = { -1.0, -1.0, 3.0, -1.0 };
static const float VERTICES_UV[] = { 0.0, 1.0, 2.0, 1.0 };

VertexOutput mainVS(uint vertexID : SV_VertexID)
{
    VertexOutput vsOutput;
    
	vsOutput.v_TexCoords = float2((vertexID << 1) & 2, vertexID & 2);
	vsOutput.v_Position = float4(vsOutput.v_TexCoords * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    
    return vsOutput;
}

float3 acesHdr2Ldr(float3 hdr)
{
	float3x3 m1 = float3x3(
		float3(0.59719f, 0.07600f, 0.02840f), 
		float3(0.35458f, 0.90834f, 0.13383f), 
		float3(0.04823f, 0.01566f, 0.83777f)
		);
	float3x3 m2 = float3x3(
		float3(1.60475f, -0.10208, -0.00327f),
		float3(-0.53108f, 1.10813, -0.07276f),
		float3(-0.07367f, -0.00605, 1.07602f)
		);

	float3 v = mul(hdr, m1);
	float3 a = v * (v + float3(0.0245786f, 0.0245786f, 0.0245786f)) - float3(0.000090537f, 0.000090537f, 0.000090537f);
	float3 b = v * (float3(0.983729f, 0.983729f, 0.983729f) * v + float3(0.4329510f, 0.4329510f, 0.4329510f)) + float3(0.238081f, 0.238081f, 0.238081f);
	float3 ldr = saturate(mul(a / b, m2));

	return ldr;
}

float3 adjustExposure(float3 color, float EV100)
{
	float LMax = (78.0f / (0.65f * 100.0f)) * pow(2.0f, EV100);
	return color * (1.0f / LMax);
}

float3 correctGamma(float3 color, float gamma)
{
	return pow(color, 1.0f / gamma);
}

cbuffer PostProcessSettigs : register(b9)
{
    float c_EV100;
    float c_Gamma;
}

Texture2D<float4> g_TextureHDR : register(t0);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
	float4 color = g_TextureHDR.Sample(g_TrilinearClamp, psInput.v_TexCoords);
    color.xyz = adjustExposure(color.xyz, c_EV100);
    color.xyz = acesHdr2Ldr(color.xyz);
    color.xyz = correctGamma(color.xyz, c_Gamma);
    return color;
}
