#include "Include/Common.hlsli"
#include "Include/EnvironmentMapping.hlsli"

cbuffer EnvironmentMapping : register(b0)
{
    uint c_EnvironmentMapSize;
    uint c_SamplesCount;
};

struct VertexOutput
{
    float4 v_Position  : SV_POSITION;
    float2 v_TexCoords : TEXCOORDS;
};

VertexOutput mainVS(uint vertexID : SV_VertexID)
{
    VertexOutput vsOutput;
    
	vsOutput.v_TexCoords = float2((vertexID << 1) & 2, vertexID & 2);
	vsOutput.v_Position = float4(vsOutput.v_TexCoords * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    
    return vsOutput;
}

float G1(float NdotV, float roughness)
{
    const float k = roughness * roughness / 2.0;

    const float nom   = NdotV;
    const float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float G_Smith(float NdotV, float NdotL, float roughness)
{
    float ggx2 = G1(NdotV, roughness);
    float ggx1 = G1(NdotL, roughness);

    return ggx1 * ggx2;
}

float2 mainPS(VertexOutput psInput) : SV_TARGET
{
    const float roughness = psInput.v_TexCoords.x;
    const float NdotV = psInput.v_TexCoords.y;

    const float3 view = float3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);

    float A = 0.0;
    float B = 0.0;
    uint usedSamples = 0;
    for (uint i = 0; i < c_SamplesCount; ++i)
    {
        const float2 Xi = RandomHammersley(i, c_SamplesCount);
        const float3 H = ImportanceSampleGGX(Xi, roughness);
        
        const float VdotH = saturate(dot(view, H));
        const float3 L = 2.0 * VdotH * H - view;

        const float NdotL = saturate(L.z);
        const float NdotH = saturate(H.z);

        if (NdotL > 0.0)
        {
            const float G = G_Smith(NdotV, NdotL, roughness);

            const float G_Vis = G * VdotH / (NdotH * NdotV);
            const float Fc = pow(1.0 - VdotH, 5.0);
            
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
            
            ++usedSamples;
        }
    }

    return float2(A, B) / float(usedSamples);
}