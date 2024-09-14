#include "Include/Buffers.hlsli"
#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"

struct VertexInput
{
    float3 a_Position  : POSITION;
    float3 a_Normal    : NORMAL;
    float3 a_Tangent   : TANGENT;
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
    float4                v_Position          : SV_POSITION;
    float2                v_TexCoords         : TEXCOORDS;
    nointerpolation float v_DissolutionFactor : DISSOLUTION_FACTOR;
};

VertexOutput mainVS(VertexInput vsInput, TransformInput transformInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float3 vertexPos = mul(float4(vsInput.a_Position, 1.0), transformInput.a_Transform).xyz;
    
    vsOutput.v_Position = mul(float4(vertexPos, 1.0), c_ViewProjection);
    vsOutput.v_TexCoords = vsInput.a_TexCoords;
    vsOutput.v_DissolutionFactor = saturate(instInput.a_ElapsedTime / instInput.a_DissolutionDuration);

    return vsOutput;
}

Texture2D<float> t_DissolutionNoiseMap : register(t20);

void mainPS(VertexOutput psInput)
{
    const float dissolutionNoise = t_DissolutionNoiseMap.Sample(s_NearestWrap, psInput.v_TexCoords).r;
    
    const float dissolutionEpsilon = 0.15; // To prevent dissolution dash when dissolution noise is 1.0
    const float fragmentDissolution = psInput.v_DissolutionFactor + dissolutionEpsilon;
    
    clip(fragmentDissolution - dissolutionNoise);
}