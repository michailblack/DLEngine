#include "GlobalCB.hlsl"

cbuffer MeshInstance : register(b2)
{
    float4x4 c_MeshToModel;
    float4x4 c_ModelToMesh;
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
    float4x4 a_ModelToWorld : TRANSFORM;
    float _emptyInstance    : _empty;
};

struct VertexOutput
{
    float4 v_Position : SV_POSITION;
    float3 v_WorldPos : WORLD_POS;
};

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    float4 worldPos = mul(float4(vsInput.a_Position, 1.0), c_MeshToModel);
    worldPos = mul(worldPos, instInput.a_ModelToWorld);

    vsOutput.v_WorldPos = worldPos.xyz;
    vsOutput.v_Position = mul(worldPos, c_ViewProjection);

    return vsOutput;
}

struct HullPatchOutput
{
	float EdgeTessFactor[3]	: SV_TessFactor;
	float InsideTessFactor	: SV_InsideTessFactor;
};

static const uint NUM_CONTROL_POINTS = 3;
static const float MAX_TESS_FACTOR = 64.0;
static const float MIN_TESS_FACTOR = 1.0;
static const float SCALING_CONSTANT = 0.5;

float CalcTessFactor(float edgeLength)
{
    return min(MAX_TESS_FACTOR, MIN_TESS_FACTOR + SCALING_CONSTANT * edgeLength);
}

HullPatchOutput CalcHSPatchConstants(
    InputPatch<VertexOutput,
    NUM_CONTROL_POINTS> inputPatch,
    uint patchID : SV_PrimitiveID
)
{
	HullPatchOutput patchOutput;

	float AB = length(inputPatch[1].v_WorldPos - inputPatch[0].v_WorldPos);
    float BC = length(inputPatch[2].v_WorldPos - inputPatch[1].v_WorldPos);
    float CA = length(inputPatch[0].v_WorldPos - inputPatch[2].v_WorldPos);
    float maxEdgeLength = max(AB, max(BC, CA));

	patchOutput.EdgeTessFactor[0] = CalcTessFactor(AB);
	patchOutput.EdgeTessFactor[1] = CalcTessFactor(BC);
    patchOutput.EdgeTessFactor[2] = CalcTessFactor(CA);
    patchOutput.InsideTessFactor  = CalcTessFactor(maxEdgeLength);

	return patchOutput;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(MAX_TESS_FACTOR)]
VertexOutput mainHS(
    InputPatch<VertexOutput,
    NUM_CONTROL_POINTS> inputPatch,
    uint pointID : SV_OutputControlPointID,
    uint patchID : SV_PrimitiveID
)
{
	return inputPatch[pointID];
}

[domain("tri")]
VertexOutput mainDS(
    HullPatchOutput patchConstantData,
    float3 bary : SV_DomainLocation,
    const OutputPatch<VertexOutput, NUM_CONTROL_POINTS> patch
)
{
	VertexOutput dsOutput;

    dsOutput.v_Position  = bary.x * patch[0].v_Position  + bary.y * patch[1].v_Position  + bary.z * patch[2].v_Position;
    dsOutput.v_WorldPos  = bary.x * patch[0].v_WorldPos  + bary.y * patch[1].v_WorldPos  + bary.z * patch[2].v_WorldPos;

	return dsOutput;
}

struct GeometryOutput
{
	float4 v_Position : SV_POSITION;
};

static const float NORMAL_LENGTH = 0.015;

[maxvertexcount(2)]
void mainGS(triangle VertexOutput input[3], inout LineStream<GeometryOutput> output)
{
    float3 edge1 = input[1].v_WorldPos - input[0].v_WorldPos;
    float3 edge2 = input[2].v_WorldPos - input[0].v_WorldPos;
    float3 triangleNormal = normalize(cross(edge1, edge2));
	
    float3 triangleCenter = (input[0].v_WorldPos + input[1].v_WorldPos + input[2].v_WorldPos) / 3.0;
    
    GeometryOutput vertex;
    vertex.v_Position = mul(float4(triangleCenter, 1.0), c_ViewProjection);
    output.Append(vertex);
    
    vertex.v_Position = mul(float4(triangleCenter + triangleNormal * NORMAL_LENGTH, 1.0), c_ViewProjection);
    output.Append(vertex);
}

float4 mainPS(GeometryOutput psInput) : SV_TARGET
{
    return float4(1.0, 1.0, 0.0, 1.0);
}
