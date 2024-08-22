#include "Include/Buffers.hlsli"
#include "Include/Common.hlsli"
#include "Include/EnvironmentMapping.hlsli"
#include "Include/PBR.hlsli"

cbuffer EnvironmentMapping : register(b0)
{
    uint  c_EnvironmentMapSize;
    uint  c_SamplesCount;
    float c_Roughness;
};

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
    vsOutput.v_CubemapSample = float3(vsOutput.v_Position.xy, 1.0);
    
    return vsOutput;
}

struct GeometryOutput
{
    float4 v_Position               : SV_POSITION;
    float3 v_CubemapSample          : CUBEMAP_SAMPLE;
    uint   v_RenderTargetArrayIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(18)]
void mainGS(triangle VertexOutput input[3], inout TriangleStream<GeometryOutput> output)
{
    for (uint faceIndex = 0; faceIndex < 5; ++faceIndex)
    {
        for (uint vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
        {
            GeometryOutput outputVertex;
            outputVertex.v_Position = input[vertexIndex].v_Position;
            outputVertex.v_CubemapSample = RotateToNewBasis(input[vertexIndex].v_CubemapSample, CubeFaceNormals[faceIndex]);
            outputVertex.v_RenderTargetArrayIndex = faceIndex;
            output.Append(outputVertex);
        }
        output.RestartStrip();
    }

    // When rotating for -Z face, the texture is upside down
    // So it is handled separately
    for (uint vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
    {
        const float3 rotatedCubemapSample = RotateToNewBasis(input[vertexIndex].v_CubemapSample, CubeFaceNormals[5]);
        GeometryOutput outputVertex;
        outputVertex.v_Position = input[vertexIndex].v_Position;
        outputVertex.v_CubemapSample = float3(-rotatedCubemapSample.x, -rotatedCubemapSample.y, rotatedCubemapSample.z);
        outputVertex.v_RenderTargetArrayIndex = 5;
        output.Append(outputVertex);
    }
}

SamplerState s_Anisotropic8Clamp : register(s0);

TextureCube t_EnvironmentMap : register(t0);

float3 mainPS(GeometryOutput psInput) : SV_TARGET
{
    const float3 normal = normalize(psInput.v_CubemapSample);
    const float3x3 rotation = BranchlessONB(normal);

    uint usedSamples = 0;
    float3 irradiance = float3(0.0, 0.0, 0.0);
    for (uint i = 0; i < c_SamplesCount; ++i)
    {
        float NdotH;
        const float3 H = ImportanceSampleGGX(NdotH, i, c_SamplesCount, c_Roughness, rotation);
        const float3 location = reflect(-normal, H);

        if (dot(normal, normalize(location)) < Epsilon)
            continue;

        const float sizeFactor = 2.0 / (PI * c_SamplesCount * NDF_GGX(NdotH, c_Roughness));
        const float mipLevel = HemisphereMip(sizeFactor, c_EnvironmentMapSize);

        irradiance += t_EnvironmentMap.SampleLevel(s_Anisotropic8Clamp, location, mipLevel).rgb;
        ++usedSamples;
    }

    return irradiance /= float(usedSamples);
}
