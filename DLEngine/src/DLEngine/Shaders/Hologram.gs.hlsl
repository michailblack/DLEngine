cbuffer PerFrame : register(b0)
{
	float c_TimeMS;
	float c_TimeS;
	float2 c_Resolution;
	float2 c_MousePos;
};

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

struct GeometryInput
{
	float4 v_Position  : SV_POSITION;
	float3 v_WorldPos  : WORLD_POS;
	float3 v_Normal    : NORMAL;
	float3 v_BaseColor : BASE_COLOR;
	float3 v_AddColor  : ADD_COLOR;
};

struct GeometryOutput
{
	float4 o_Position  : SV_POSITION;
	float3 o_WorldPos  : WORLD_POS;
	float3 o_Normal    : NORMAL;
	float3 o_BaseColor : BASE_COLOR;
	float3 o_AddColor  : ADD_COLOR;
};

#include "Hologram.h.hlsl"

float3 vertexDistortion(float3 pos, float3 normal)
{
	float3 offset = 0.0;
	offset += normal * 0.025 * wave(pos, BASE_WAVE_INTERVAL, BASE_WAVE_SPEED, BASE_WAVE_THICKNESS, true);
	offset += normal * 0.05 * wave(pos, ADD_WAVE_INTERVAL, ADD_WAVE_SPEED, ADD_WAVE_THICKNESS, false);
	return offset;
}

[maxvertexcount(3)]
void main(triangle GeometryInput input[3], inout TriangleStream<GeometryOutput> output)
{
	float3 edge1 = input[1].v_WorldPos - input[0].v_WorldPos;
	float3 edge2 = input[2].v_WorldPos - input[0].v_WorldPos;
	float3 triangleNormal = normalize(cross(edge1, edge2));
	
    float3 triangleCenter = (input[0].v_WorldPos + input[1].v_WorldPos + input[2].v_WorldPos) / 3.0;
    float3 offset = vertexDistortion(triangleCenter, triangleNormal);
    
	for (uint i = 0; i < 3; i++)
	{
		GeometryOutput gsOutput;
		
        gsOutput.o_WorldPos = input[i].v_WorldPos + offset;
        gsOutput.o_Position = mul(float4(gsOutput.o_WorldPos, 1.0), c_ViewProjection);
		
        gsOutput.o_Normal    = input[i].v_Normal;
        gsOutput.o_BaseColor = input[i].v_BaseColor;
        gsOutput.o_AddColor  = input[i].v_AddColor;
		        
		output.Append(gsOutput);
	}
}