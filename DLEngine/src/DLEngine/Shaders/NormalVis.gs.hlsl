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
    float4 v_Position : SV_POSITION;
    float3 v_WorldPos : WORLD_POS;
};

struct GeometryOutput
{
	float4 o_Position : SV_POSITION;
};

static const float NORMAL_LENGTH = 0.015;

[maxvertexcount(2)]
void main(triangle GeometryInput input[3], inout LineStream<GeometryOutput> output)
{
    float3 edge1 = input[1].v_WorldPos - input[0].v_WorldPos;
    float3 edge2 = input[2].v_WorldPos - input[0].v_WorldPos;
    float3 triangleNormal = normalize(cross(edge1, edge2));
	
    float3 triangleCenter = (input[0].v_WorldPos + input[1].v_WorldPos + input[2].v_WorldPos) / 3.0;
    
    GeometryOutput vertex;
    vertex.o_Position = mul(float4(triangleCenter, 1.0), c_ViewProjection);
    output.Append(vertex);
    
    vertex.o_Position = mul(float4(triangleCenter + triangleNormal * NORMAL_LENGTH, 1.0), c_ViewProjection);
    output.Append(vertex);
}
