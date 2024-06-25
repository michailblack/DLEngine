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
    float4 c_FrustumTopRightDir;
    float4 c_FrustumTopLeftDir;
    float4 c_FrustumBottomLeftDir;
    float4 c_FrustumBottomRightDir;
};

SamplerState g_PointWrap : register(s0);
SamplerState g_PointClamp : register(s1);

SamplerState g_TrilinearWrap : register(s2);
SamplerState g_TrilinearClamp : register(s3);

SamplerState g_AnisotropicWrap : register(s4);
SamplerState g_AnisotropicClamp : register(s5);

SamplerState g_ActiveSampler : register(s6);
