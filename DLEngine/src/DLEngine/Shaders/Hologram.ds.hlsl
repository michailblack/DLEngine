cbuffer PerView : register(b1)
{
    float4x4 c_Projection;
    float4x4 c_InvProjection;
    float4x4 c_View;
    float4x4 c_InvView;
    float4x4 c_ViewProjection;
    float4x4 c_InvViewProjection;
    float4 c_CameraPosition;
};

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
DomainOutput main(
	HullPatchOutput patchConstantData,
	float3 bary : SV_DomainLocation,
	const OutputPatch<DomainInput, NUM_CONTROL_POINTS> patch)
{
	DomainOutput dsOutput;

    dsOutput.o_WorldPos  = bary.x * patch[0].v_WorldPos  + bary.y * patch[1].v_WorldPos  + bary.z * patch[2].v_WorldPos;

    dsOutput.o_Position  = mul(float4(dsOutput.o_WorldPos, 1.0), c_ViewProjection);

    dsOutput.o_Normal    = bary.x * patch[0].v_Normal    + bary.y * patch[1].v_Normal    + bary.z * patch[2].v_Normal;
    dsOutput.o_BaseColor = bary.x * patch[0].v_BaseColor + bary.y * patch[1].v_BaseColor + bary.z * patch[2].v_BaseColor;
    dsOutput.o_AddColor  = bary.x * patch[0].v_AddColor  + bary.y * patch[1].v_AddColor  + bary.z * patch[2].v_AddColor;

	return dsOutput;
}
