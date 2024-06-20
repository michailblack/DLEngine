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
    float4 c_BL;
    float4 c_BL2TL;
    float4 c_BL2BR;
};

static const uint MAX_CONSTANT_BUFFER_FLOAT4_ELEMENTS_COUNT = 4096u;

cbuffer Transforms : register(b2)
{
    float4x4 c_ModelToWorld[MAX_CONSTANT_BUFFER_FLOAT4_ELEMENTS_COUNT / 4u];
};

cbuffer InvTransforms : register(b3)
{
    float4x4 c_WorldToModel[MAX_CONSTANT_BUFFER_FLOAT4_ELEMENTS_COUNT / 4u];
};

cbuffer MeshInstance : register(b4)
{
    float4x4 c_MeshToModel;
    float4x4 c_ModelToMesh;
};
