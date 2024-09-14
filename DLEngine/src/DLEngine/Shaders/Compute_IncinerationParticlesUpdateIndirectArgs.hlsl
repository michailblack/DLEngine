#include "Include/IncinerationParticle.hlsli"

RWBuffer<uint> u_IncinerationParticleRangeBuffer : register(u1);

[numthreads(1, 1, 1)]
void mainCS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x != 0)
        return;
    
    const uint particlesCount = u_IncinerationParticleRangeBuffer[ParticlesCountIndex];
    const uint expiredParticlesCount = u_IncinerationParticleRangeBuffer[ExpiredParticlesCountIndex];
    
    const uint aliveParticlesCount = particlesCount - expiredParticlesCount;
    const uint threadGroupsCountX = aliveParticlesCount / 64;
    
    u_IncinerationParticleRangeBuffer[ThreadGroupCountXIndex] = aliveParticlesCount % 64 == 0 ? threadGroupsCountX : threadGroupsCountX + 1;
    u_IncinerationParticleRangeBuffer[ThreadGroupCountXIndex + 1] = 1;
    u_IncinerationParticleRangeBuffer[ThreadGroupCountXIndex + 2] = 1;
}