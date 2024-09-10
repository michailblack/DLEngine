#include "Include/Buffers.hlsli"
#include "Include/Samplers.hlsli"

#define FXAA_PC 1 // PC platform
#define FXAA_HLSL_5 1 // Shader Model 5
#define FXAA_QUALITY__PRESET 39
    // 10 to 15 - default medium dither (10=fastest, 15=highest quality)
    // 20 to 29 - less dither, more expensive (20=fastest, 29=highest quality)
    // 39       - no dither, very expensive
#include "Include/Fxaa3_11.hlsl"

cbuffer PostProcessing : register(b5)
{
    float c_EV100;
    float c_Gamma;
    float c_FXAA_QualitySubpix;
    float c_FXAA_QualityEdgeThreshold;
    float c_FXAA_QualityEdgeThresholdMin;
}

struct VertexOutput
{
    float4 v_Position : SV_POSITION;
};

VertexOutput mainVS(uint vertexID : SV_VertexID)
{
    VertexOutput vsOutput;
    
    const float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    vsOutput.v_Position = float4(uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    
    return vsOutput;
}

Texture2D<float4> t_LDR_ResolveTexture : register(t20);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    FxaaTex TextureAndSampler;
    TextureAndSampler.tex = t_LDR_ResolveTexture;
    TextureAndSampler.smpl = s_BilinearClamp;
    TextureAndSampler.UVMinMax = float4(0.0, 0.0, 1.0, 1.0); // fullscreen uv
    
    const float2 invViewportSize = float2(c_InvViewportWidth, c_InvViewportHeight);
    
    return FxaaPixelShader(
		psInput.v_Position.xy * invViewportSize, // map pixel coordinate to [0; 1] range
		0, // unused, for consoles
		TextureAndSampler,
		TextureAndSampler,
		TextureAndSampler,
		invViewportSize,
		0, // unused, for consoles
		0, // unused, for consoles
		0, // unused, for consoles
		c_FXAA_QualitySubpix,
		c_FXAA_QualityEdgeThreshold,
		c_FXAA_QualityEdgeThresholdMin,
		0, // unused, for consoles
		0, // unused, for consoles
		0, // unused, for consoles
		0  // unused, for consoles
	);
}
