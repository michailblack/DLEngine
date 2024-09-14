#include "Include/GBufferResources.hlsli"
#include "Include/Lighting.hlsli"

struct VertexOutput
{
    float4 v_Position  : SV_POSITION;
    float2 v_TexCoords : TEXCOORDS;
};

VertexOutput mainVS(uint vertexID : SV_VertexID)
{
    VertexOutput vsOutput;
    
    vsOutput.v_TexCoords = float2((vertexID << 1) & 2, vertexID & 2);
    vsOutput.v_Position = float4(vsOutput.v_TexCoords * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    
    return vsOutput;
}

Texture2D<float> t_DepthBufferCopy : register(t20);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    const float3 albedo                 = t_GBufferAlbedo.Sample(s_TrilinearClamp, psInput.v_TexCoords).rgb;
    const float2 metalnessRoughness     = t_GBufferMetalnessRoughness.Sample(s_TrilinearClamp, psInput.v_TexCoords).rg;
    const float4 geometrySurfaceNormals = t_GBufferGeometrySurfaceNormals.Sample(s_NearestClamp, psInput.v_TexCoords);
    const float3 emission               = t_GBufferEmission.Sample(s_TrilinearClamp, psInput.v_TexCoords).rgb;

    float rough = metalnessRoughness.g;
    if (c_OverwriteRoughness)
        rough = c_OverwrittenRoughness;

    rough = max(rough, 0.01); // To keep specular highlight for small values

    Surface surface;
    surface.Albedo = albedo;
    surface.GeometryNormal = unpackOctahedron(geometrySurfaceNormals.rg);
    surface.SurfaceNormal = unpackOctahedron(geometrySurfaceNormals.ba);
    surface.Metalness = metalnessRoughness.r;
    surface.Roughness = rough;
    surface.F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalnessRoughness.r);
    
    const float ndcX = psInput.v_TexCoords.x *  2.0 - 1.0;
    const float ndcY = psInput.v_TexCoords.y * -2.0 + 1.0;
    const float depth = t_DepthBufferCopy.Sample(s_NearestClamp, psInput.v_TexCoords).r;
    
    float4 worldPos = mul(float4(ndcX, ndcY, depth, 1.0), c_InvViewProjection);
    worldPos /= worldPos.w;

    View view;
    view.ViewDir = normalize(c_CameraPosition - worldPos.xyz);
    view.ReflectionDir = normalize(reflect(-view.ViewDir, surface.SurfaceNormal));
    view.NoV = max(dot(surface.SurfaceNormal, view.ViewDir), Epsilon);
    view.NoL = -1.0;

    float3 indirectLighting = IBL(view, surface);
    if (!c_UseIBL)
        indirectLighting = c_IndirectLightingRadiance * albedo;

    const float3 directLighting = CalculateDirectLighting(view, surface, worldPos.xyz);

    return float4(indirectLighting + directLighting + emission, 1.0);
}