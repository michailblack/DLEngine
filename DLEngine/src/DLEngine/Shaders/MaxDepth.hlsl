cbuffer PostProcessing : register(b4)
{
    uint c_SamplesCount;
    float c_EV100;
    float c_Gamma;
}

struct VertexOutput
{
    float4 v_Position : SV_POSITION;
    float2 v_TexCoords : TEXCOORDS;
};

VertexOutput mainVS(uint vertexID : SV_VertexID)
{
    VertexOutput vsOutput;
    
    vsOutput.v_TexCoords = float2((vertexID << 1) & 2, vertexID & 2);
    vsOutput.v_Position = float4(vsOutput.v_TexCoords * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    
    return vsOutput;
}

Texture2DMS<float> t_SceneDepth : register(t16);

float mainPS(VertexOutput psInput) : SV_Depth
{
    float maxDepth = 0.0f;
    for (uint i = 0; i < c_SamplesCount; ++i)
    {
        float depth = t_SceneDepth.Load(int2(psInput.v_Position.xy), i).r;
        maxDepth = max(maxDepth, depth);
    }

    return maxDepth;
}
