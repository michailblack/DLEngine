struct DomainInput
{
	float4 v_Position  : SV_POSITION;
    float3 v_WorldPos  : WORLD_POS;
    float3 v_Normal    : NORMAL;
    float3 v_BaseColor : BASE_COLOR;
    float3 v_AddColor  : ADD_COLOR;
};

struct DomainOutput
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

[domain("tri")]
DomainOutput main(HullPatchOutput patchConstantData, float3 bary : SV_DomainLocation, const OutputPatch<DomainInput, NUM_CONTROL_POINTS> patch)
{
	DomainOutput dsOutput;

    dsOutput.o_Position  = bary.x * patch[0].v_Position  + bary.y * patch[1].v_Position  + bary.z * patch[2].v_Position;
    dsOutput.o_WorldPos  = bary.x * patch[0].v_WorldPos  + bary.y * patch[1].v_WorldPos  + bary.z * patch[2].v_WorldPos;
    dsOutput.o_Normal    = bary.x * patch[0].v_Normal    + bary.y * patch[1].v_Normal    + bary.z * patch[2].v_Normal;
    dsOutput.o_BaseColor = bary.x * patch[0].v_BaseColor + bary.y * patch[1].v_BaseColor + bary.z * patch[2].v_BaseColor;
    dsOutput.o_AddColor  = bary.x * patch[0].v_AddColor  + bary.y * patch[1].v_AddColor  + bary.z * patch[2].v_AddColor;

	return dsOutput;
}
