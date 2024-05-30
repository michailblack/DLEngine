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

cbuffer PerDraw : register(b2)
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
    float3 a_BaseColor      : BASE_COLOR;
    float3 a_AddColor       : ADD_COLOR;
};

struct VertexOutput
{
    float4 o_Position  : SV_POSITION;
    float3 o_WorldPos  : WORLD_POS;
    float3 o_Normal    : NORMAL;
    float3 o_BaseColor : BASE_COLOR;
    float3 o_AddColor  : ADD_COLOR;
};

VertexOutput main(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    float4x4 meshToWorld = mul(c_MeshToModel, instInput.a_ModelToWorld);
    float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), meshToWorld);

    vsOutput.o_Position = mul(vertexPos, c_ViewProjection);
    vsOutput.o_WorldPos = float3(vertexPos.xyz);

    float3 axisX = normalize(meshToWorld[0].xyz);
    float3 axizY = normalize(meshToWorld[1].xyz);
    float3 axizZ = normalize(meshToWorld[2].xyz);
    vsOutput.o_Normal = vsInput.a_Normal.x * axisX + vsInput.a_Normal.y * axizY + vsInput.a_Normal.z * axizZ;

    vsOutput.o_BaseColor = instInput.a_BaseColor;
    vsOutput.o_AddColor = instInput.a_AddColor;

    return vsOutput;
}
