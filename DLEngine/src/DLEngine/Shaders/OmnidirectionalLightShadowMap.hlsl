cbuffer OmnidirectionalLightShadowData : register(b4)
{
    float4x4 c_ViewProjections[6];
    float3   c_LightPosition;
    float    c_ShadowMapWorldTexelSize;
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

static const float Sqrt2 = sqrt(2);

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float3 vertexPos = mul(float4(vsInput.a_Position, 1.0), instInput.a_Transform).xyz;

    float3 axisX = normalize(instInput.a_Transform[0].xyz);
    float3 axizY = normalize(instInput.a_Transform[1].xyz);
    float3 axizZ = normalize(instInput.a_Transform[2].xyz);
    const float3 geometryNormal = normalize(vsInput.a_Normal.x * axisX + vsInput.a_Normal.y * axizY + vsInput.a_Normal.z * axizZ);

    const float3 lightDir = normalize(vertexPos - c_LightPosition);
    const float3 offset = c_ShadowMapWorldTexelSize / Sqrt2 * (geometryNormal - 0.9 * lightDir * dot(geometryNormal, lightDir));

    vsOutput.v_WorldPos = vertexPos - offset;

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