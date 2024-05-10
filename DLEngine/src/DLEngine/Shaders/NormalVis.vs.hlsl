cbuffer PerView : register(b1)
{
    row_major float4x4 c_Projection;
    row_major float4x4 c_InvProjection;
    row_major float4x4 c_View;
    row_major float4x4 c_InvView;
    row_major float4x4 c_ViewProjection;
    row_major float4x4 c_InvViewProjection;
    float4 c_CameraPosition;
};

cbuffer PerDraw : register(b2)
{
    row_major float4x4 c_MeshToModel;
    row_major float4x4 c_ModelToMesh;
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
    row_major float4x4 a_ModelToWorld : TRANSFORM;
};

struct VertexOutput
{
    float4 o_Position : SV_POSITION;
    float3 o_Normal   : NORMAL;
};

VertexOutput main(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    float4x4 meshToWorld = mul(c_MeshToModel, instInput.a_ModelToWorld);
    
    vsOutput.o_Position = mul(float4(vsInput.a_Position, 1.0), meshToWorld);
    vsOutput.o_Position = mul(vsOutput.o_Position, c_ViewProjection);
    
    float3 axisX = normalize(meshToWorld[0].xyz);
    float3 axizY = normalize(meshToWorld[1].xyz);
    float3 axizZ = normalize(meshToWorld[2].xyz);

    vsOutput.o_Normal = vsInput.a_Normal.x * axisX + vsInput.a_Normal.y * axizY + vsInput.a_Normal.z * axizZ;

    return vsOutput;
}
