#include "Include/IncinerationParticle.hlsli"

RWStructuredBuffer<IncinerationParticle> u_IncinerationParticles           : register(u0);
RWBuffer<uint>                           u_IncinerationParticleRangeBuffer : register(u1);

[numthreads(1, 1, 1)]
void mainCS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x != 0)
        return;
    
    uint maxParticlesCount = 0, particleStride = 0;
    u_IncinerationParticles.GetDimensions(maxParticlesCount, particleStride);
    
    const uint expiredParticlesCount = u_IncinerationParticleRangeBuffer[ExpiredParticlesCountIndex];
    const uint newParticlesCount = u_IncinerationParticleRangeBuffer[ParticlesCountIndex] - expiredParticlesCount;
    const uint previousParticlesOffset = u_IncinerationParticleRangeBuffer[ParticleOffsetIndex];
    
    u_IncinerationParticleRangeBuffer[ParticlesCountIndex] = newParticlesCount;
    u_IncinerationParticleRangeBuffer[ParticleOffsetIndex] = (previousParticlesOffset + expiredParticlesCount) % maxParticlesCount;
    u_IncinerationParticleRangeBuffer[ExpiredParticlesCountIndex] = 0;
    
    // Draw indexed instanced indirect args
    u_IncinerationParticleRangeBuffer[ThreadGroupCountXIndex + 3] = 6;                 // IndexCountPerInstance
    u_IncinerationParticleRangeBuffer[ThreadGroupCountXIndex + 4] = newParticlesCount; // InstanceCount
    u_IncinerationParticleRangeBuffer[ThreadGroupCountXIndex + 5] = 0;                 // StartIndexLocation
    u_IncinerationParticleRangeBuffer[ThreadGroupCountXIndex + 6] = 0;                 // BaseVertexLocation
    u_IncinerationParticleRangeBuffer[ThreadGroupCountXIndex + 7] = 0;                 // StartInstanceLocation
}