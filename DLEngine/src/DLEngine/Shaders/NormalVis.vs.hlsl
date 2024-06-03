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
    float4 o_Position : SV_POSITION;
    float3 o_WorldPos : WORLD_POS;
};

VertexOutput main(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    float4 worldPos = mul(float4(vsInput.a_Position, 1.0), c_MeshToModel);
    worldPos = mul(worldPos, instInput.a_ModelToWorld);

    vsOutput.o_WorldPos = worldPos.xyz;
    vsOutput.o_Position = mul(worldPos, c_ViewProjection);

    return vsOutput;
}
