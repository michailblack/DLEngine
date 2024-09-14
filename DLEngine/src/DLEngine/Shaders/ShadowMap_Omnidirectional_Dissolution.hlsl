#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"

cbuffer OmnidirectionalLightShadowData : register(b5)
{
    float4x4 c_ViewProjections[6];
};

struct VertexInput
{
    float3 a_Position : POSITION;
    float3 a_Normal : NORMAL;
    float3 a_Tangent : TANGENT;
    float3 a_Bitangent : BITANGENT;
    float2 a_TexCoords : TEXCOORDS;
};

struct TransformInput
{
    float4x4 a_Transform : TRANSFORM;
};

struct InstanceInput
{
    uint2 a_InstanceUUID        : INSTANCE_UUID;
    float a_DissolutionDuration : DISSOLUTION_DURATION;
    float a_ElapsedTime         : ELAPSED_TIME;
};

struct VertexOutput
{
    float3                v_WorldPos          : WORLD_POS;
    float2                v_TexCoords         : TEXCOORDS;
    nointerpolation float v_DissolutionFactor : DISSOLUTION_FACTOR;
};

VertexOutput mainVS(VertexInput vsInput, TransformInput transformInput, InstanceInput instInput)
{
    VertexOutput vsOutput;
    vsOutput.v_WorldPos = mul(float4(vsInput.a_Position, 1.0), transformInput.a_Transform).xyz;
    vsOutput.v_TexCoords = vsInput.a_TexCoords;
    vsOutput.v_DissolutionFactor = saturate(instInput.a_ElapsedTime / instInput.a_DissolutionDuration);

    return vsOutput;
}

struct GeometryOutput
{
    float4                v_Position               : SV_POSITION;
    float2                v_TexCoords              : TEXCOORDS;
    uint                  v_RenderTargetArrayIndex : SV_RenderTargetArrayIndex;
    nointerpolation float v_DissolutionFactor      : DISSOLUTION_FACTOR;
};

[maxvertexcount(18)]
void mainGS(triangle VertexOutput input[3], inout TriangleStream<GeometryOutput> output)
{
    for (uint faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        for (uint vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
        {
            GeometryOutput outputVertex;
            outputVertex.v_Position = mul(float4(input[vertexIndex].v_WorldPos, 1.0), c_ViewProjections[faceIndex]);
            outputVertex.v_TexCoords = input[vertexIndex].v_TexCoords;
            outputVertex.v_RenderTargetArrayIndex = faceIndex;
            outputVertex.v_DissolutionFactor = input[vertexIndex].v_DissolutionFactor;
            output.Append(outputVertex);
        }
        output.RestartStrip();
    }
}

Texture2D<float> t_DissolutionNoiseMap : register(t20);

void mainPS(GeometryOutput psInput)
{
    const float dissolutionNoise = t_DissolutionNoiseMap.Sample(s_NearestWrap, psInput.v_TexCoords).r;
    
    const float dissolutionEpsilon = 0.15; // To prevent dissolution dash when dissolution noise is 1.0
    const float fragmentDissolution = psInput.v_DissolutionFactor + dissolutionEpsilon;
    
    clip(fragmentDissolution - dissolutionNoise);
}