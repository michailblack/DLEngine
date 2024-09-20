#include "Include/Buffers.hlsli"
#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"
#include "Include/IncinerationParticle.hlsli"

RWStructuredBuffer<IncinerationParticle> u_IncinerationParticles           : register(u0);
RWBuffer<uint>                           u_IncinerationParticleRangeBuffer : register(u1);

Texture2D<float4> t_GBuffer_GeometrySurfaceNormals : register(t20);
Texture2D<uint2>  t_GBuffer_InstanceUUID           : register(t21);
Texture2D<float>  t_GBuffer_Depth                  : register(t22);

static const float GravitionalAcceleration = 9.81; // m/s^2

float LinearizeReversedDepth(float depth)
{
    return (c_zNear * c_zFar) / lerp(c_zFar, c_zNear, depth);
}

[numthreads(64, 1, 1)]
void mainCS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= u_IncinerationParticleRangeBuffer[ParticlesCountIndex])
        return;
    
    uint maxParticlesCount = 0, particleStride = 0;
    u_IncinerationParticles.GetDimensions(maxParticlesCount, particleStride);
    
    const uint particleIndex = (u_IncinerationParticleRangeBuffer[ParticleOffsetIndex] + dispatchThreadID.x) % maxParticlesCount;
    
    IncinerationParticle particle = u_IncinerationParticles[particleIndex];
    particle.LifetimePassedMS += c_DeltaTimeMS;
    
    if (particle.LifetimePassedMS >= MaxIncinerationParticleLifetimeMS)
    {
        InterlockedAdd(u_IncinerationParticleRangeBuffer[ExpiredParticlesCountIndex], 1);
        return;
    }
    
    const float3 nextParticleWorldPos = particle.WorldPosition + particle.Velocity * c_DeltaTimeS;
    const float4 particleClipPos = mul(float4(nextParticleWorldPos, 1.0), c_ViewProjection);
    const float3 particleNDCPos = particleClipPos.xyz / particleClipPos.w;
    
    float2 uv = particleNDCPos.xy * 0.5 + float2(0.5, 0.5);
    uv.y = 1.0 - uv.y;
    
    const float sceneDepth = t_GBuffer_Depth.SampleLevel(s_NearestClamp, uv, 0.0).r;
    float4 fragmentWorldPos = mul(float4(particleNDCPos.xy, sceneDepth, 1.0), c_InvViewProjection);
    fragmentWorldPos /= fragmentWorldPos.w;
    
    if (distance(nextParticleWorldPos, fragmentWorldPos.xyz) < 2.0 * ParticlePointLightWorldRadius)
    {
        const uint2 pixelCoords = uint2(uv * float2(c_ViewportWidth, c_ViewportHeight));
        const uint2 instanceUUID = t_GBuffer_InstanceUUID.Load(int3(pixelCoords, 0)).rg;
        
        if (!all(instanceUUID == particle.ParentInstanceUUID))
        {
            const float3 surfaceNormal = unpackOctahedron(t_GBuffer_GeometrySurfaceNormals.SampleLevel(s_NearestClamp, uv, 0.0).ba);
            const float3 reflectedVelocityDir = reflect(normalize(particle.Velocity), surfaceNormal);
            const float particleImpulseMultiplier = lerp(0.1, 0.5, max(dot(reflectedVelocityDir, surfaceNormal), Epsilon));
            particle.Velocity = reflectedVelocityDir * length(particle.Velocity) * particleImpulseMultiplier;
        }
    }
    else
    {
        particle.Velocity += float3(0.0, -GravitionalAcceleration, 0.0) * c_DeltaTimeS;
    }
    
    particle.WorldPosition += particle.Velocity * c_DeltaTimeS;
    
    u_IncinerationParticles[particleIndex] = particle;
}