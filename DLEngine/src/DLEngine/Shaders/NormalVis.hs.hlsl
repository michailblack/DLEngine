struct HullInput
{
	float4 v_Position  : SV_POSITION;
    float3 v_WorldPos  : WORLD_POS;
};

struct HullOutput
{
	float4 o_Position  : SV_POSITION;
    float3 o_WorldPos  : WORLD_POS;
};

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

HullPatchOutput CalcHSPatchConstants(InputPatch<HullInput, NUM_CONTROL_POINTS> inputPatch, uint patchID : SV_PrimitiveID)
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
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(MAX_TESS_FACTOR)]
HullOutput main(InputPatch<HullInput, NUM_CONTROL_POINTS> inputPatch, uint pointID : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
	HullOutput hsOutput;

	hsOutput.o_Position  = inputPatch[pointID].v_Position;
    hsOutput.o_WorldPos  = inputPatch[pointID].v_WorldPos;

	return hsOutput;
}
