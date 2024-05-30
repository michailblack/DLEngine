struct HullInput
{
	float4 v_Position  : SV_POSITION;
    float3 v_WorldPos  : WORLD_POS;
    float3 v_Normal    : NORMAL;
    float3 v_BaseColor : BASE_COLOR;
    float3 v_AddColor  : ADD_COLOR;
};

struct HullOutput
{
	float4 o_Position  : SV_POSITION;
    float3 o_WorldPos  : WORLD_POS;
    float3 o_Normal    : NORMAL;
    float3 o_BaseColor : BASE_COLOR;
    float3 o_AddColor  : ADD_COLOR;
};

struct HullPatchOutput
{
	float EdgeTessFactor[3]	: SV_TessFactor;
	float InsideTessFactor	: SV_InsideTessFactor;
};

static const uint NUM_CONTROL_POINTS = 3;
static const float MAX_TESS_FACTOR = 16.0;

HullPatchOutput CalcHSPatchConstants(
	InputPatch<HullInput, NUM_CONTROL_POINTS> inputPatch,
	uint patchID : SV_PrimitiveID)
{
	HullPatchOutput patchOutput;

	float AB = length(inputPatch[1].v_WorldPos - inputPatch[0].v_WorldPos);
    float BC = length(inputPatch[2].v_WorldPos - inputPatch[1].v_WorldPos);
    float CA = length(inputPatch[0].v_WorldPos - inputPatch[2].v_WorldPos);
    float maxEdgeLength = max(AB, max(BC, CA));

	patchOutput.EdgeTessFactor[0] = AB / maxEdgeLength * MAX_TESS_FACTOR;
	patchOutput.EdgeTessFactor[1] = BC / maxEdgeLength * MAX_TESS_FACTOR;
	patchOutput.EdgeTessFactor[2] = CA / maxEdgeLength * MAX_TESS_FACTOR;
	patchOutput.InsideTessFactor  = min(
		(patchOutput.EdgeTessFactor[0] + patchOutput.EdgeTessFactor[1] + patchOutput.EdgeTessFactor[2]) / 3.0,
        MAX_TESS_FACTOR
	);

	return patchOutput;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(MAX_TESS_FACTOR)]
HullOutput main( 
	InputPatch<HullInput, NUM_CONTROL_POINTS> inputPatch, 
	uint pointID : SV_OutputControlPointID,
	uint patchID : SV_PrimitiveID
)
{
	HullOutput hsOutput;

	hsOutput.o_Position  = inputPatch[pointID].v_Position;
    hsOutput.o_WorldPos  = inputPatch[pointID].v_WorldPos;
    hsOutput.o_Normal    = inputPatch[pointID].v_Normal;
    hsOutput.o_BaseColor = inputPatch[pointID].v_BaseColor;
    hsOutput.o_AddColor  = inputPatch[pointID].v_AddColor;

	return hsOutput;
}
