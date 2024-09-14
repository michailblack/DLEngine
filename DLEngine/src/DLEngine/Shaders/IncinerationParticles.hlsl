#include "Include/IncinerationParticle.hlsli"
#include "Include/Lighting.hlsli"

struct VertexOutput
{
    float4                 v_Position         : SV_POSITION;
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
    
    const float2 billboardVertexDelta = ParticleBillboardWorldSize * BillboardVertexDirections[vertexID % 4] * 0.5;
    
    const float3 cameraRight = normalize(c_InvView[0].xyz);
    const float3 cameraUp = normalize(c_InvView[1].xyz);
    const float3 cameraForward = normalize(c_InvView[2].xyz);
    
    const float3 billboardVertexWorldPos = particle.WorldPosition + cameraRight * billboardVertexDelta.x + cameraUp * billboardVertexDelta.y;

    VertexOutput vsOutput;
    vsOutput.v_Position = mul(float4(billboardVertexWorldPos, 1.0), c_ViewProjection);
    vsOutput.v_Emisiion = particle.Emission;
    vsOutput.v_LifetimeFactor = max(particle.LifetimePassedMS / particle.LifetimeMS, 0.0);
    
    return vsOutput;
}

Texture2D<float> t_SparkTexture : register(t20);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    clip(1.0 - psInput.v_LifetimeFactor);
    
    const float2 uv = psInput.v_Position.xy * float2(c_InvViewportWidth, c_InvViewportHeight);
    const float particleTextureGrayscale = t_SparkTexture.Sample(s_TrilinearClamp, uv).r;
    
    return float4(psInput.v_Emisiion, saturate(particleTextureGrayscale));
}