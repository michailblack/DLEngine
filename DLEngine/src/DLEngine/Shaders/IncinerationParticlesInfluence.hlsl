#include "Include/GBufferResources.hlsli"
#include "Include/IncinerationParticle.hlsli"
#include "Include/Lighting.hlsli"

struct VertexOutput
{
    float4                 v_Position         : SV_POSITION;
    float3                 v_ParticleWorldPos : PARTICLE_WORLD_POS;
    nointerpolation float3 v_Emisiion         : EMISSION;
    nointerpolation float  v_LifetimeFactor   : LIFETIME_FACTOR;
};

StructuredBuffer<IncinerationParticle> u_IncinerationParticles           : register(t21);
Buffer<uint>                           u_IncinerationParticleRangeBuffer : register(t22);

VertexOutput mainVS(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    uint maxParticlesCount = 0, particleStride = 0;
    u_IncinerationParticles.GetDimensions(maxParticlesCount, particleStride);
    
    const uint particleIndex = (u_IncinerationParticleRangeBuffer[ParticleOffsetIndex] + instanceID) % maxParticlesCount;
    const IncinerationParticle particle = u_IncinerationParticles[particleIndex];
    
    const float2 billboardVertexDelta = PartilceInfluenceBillboardWorldSize * BillboardVertexDirections[vertexID % 4] * 0.5;
    
    const float3 cameraRight = normalize(c_InvView[0].xyz);
    const float3 cameraUp = normalize(c_InvView[1].xyz);
    const float3 cameraForward = normalize(c_InvView[2].xyz);
    
    const float3 billboardVertexWorldPos = particle.WorldPosition + cameraRight * billboardVertexDelta.x + cameraUp * billboardVertexDelta.y;

    VertexOutput vsOutput;
    vsOutput.v_Position = mul(float4(billboardVertexWorldPos, 1.0), c_ViewProjection);
    vsOutput.v_ParticleWorldPos = particle.WorldPosition;
    vsOutput.v_Emisiion = particle.Emission;
    vsOutput.v_LifetimeFactor = max(particle.LifetimePassedMS / particle.LifetimeMS, 0.0);
    
    return vsOutput;
}

Texture2D<float> t_DepthBufferCopy : register(t20);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    clip(1.0 - psInput.v_LifetimeFactor);
    
    const float2 uv = psInput.v_Position.xy * float2(c_InvViewportWidth, c_InvViewportHeight);
    
    const float3 albedo                 = t_GBufferAlbedo.Sample(s_TrilinearClamp, uv).rgb;
    const float2 metalnessRoughness     = t_GBufferMetalnessRoughness.Sample(s_TrilinearClamp, uv).rg;
    const float4 geometrySurfaceNormals = t_GBufferGeometrySurfaceNormals.Sample(s_NearestClamp, uv);
    
    float rough = metalnessRoughness.g;
    if (c_OverwriteRoughness)
        rough = c_OverwrittenRoughness;

    rough = max(rough, 0.01); // To keep specular highlight for small values

    const float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalnessRoughness.r);
    
    const float ndcX = uv.x * 2.0 - 1.0;
    const float ndcY = uv.y * -2.0 + 1.0;
    const float depth = t_DepthBufferCopy.Sample(s_NearestClamp, uv).r;
    
    float4 worldPos = mul(float4(ndcX, ndcY, depth, 1.0), c_InvViewProjection);
    worldPos /= worldPos.w;

    const float3 fragmentToLight = psInput.v_ParticleWorldPos - worldPos.xyz;
    const float NoL = max(dot(unpackOctahedron(geometrySurfaceNormals.rg), normalize(fragmentToLight)), Epsilon);
    const float particlePointLightSolidAngle = CalculateSphereSolidAngle(ParticlePointLightWorldRadius, max(length(fragmentToLight), ParticlePointLightWorldRadius));
    
    float3 diffuseComponent = particlePointLightSolidAngle * albedo * (1.0 - metalnessRoughness.r) * (1.0 - FresnelSchlick(F0, NoL)) * NoL / PI;
    if (!c_UseDiffuseReflectoins)
        diffuseComponent = float3(0.0, 0.0, 0.0);
    
    return float4(diffuseComponent * psInput.v_Emisiion, 1.0);
}