#include "Include/Common.hlsli"

cbuffer OmnidirectionalLightShadowData : register(b4)
{
    float4x4 c_ViewProjections[6];
};

struct VertexInput
{
    float3 a_Position  : POSITION;
    float3 a_Normal    : NORMAL;
    float3 a_Tangent   : TANGENT;
    float3 a_Bitangent : BITANGENT;
    float2 a_TexCoords : TEXCOORDS;
};

struct InstanceInput
{
    float4x4 a_Transform : TRANSFORM;
};

struct VertexOutput
{
    float3 v_WorldPos : WORLD_POS;
};

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;
    vsOutput.v_WorldPos = mul(float4(vsInput.a_Position, 1.0), instInput.a_Transform).xyz;;

    return vsOutput;
}

struct GeometryOutput
{
    float4 v_Position               : SV_POSITION;
    uint   v_RenderTargetArrayIndex : SV_RenderTargetArrayIndex;
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
            outputVertex.v_RenderTargetArrayIndex = faceIndex;
            output.Append(outputVertex);
        }
        output.RestartStrip();
    }
}