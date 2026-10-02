#include "ParticleCommon.hlsli"

cbuffer SimulationConstants : register(b0)
{
    float deltaTime;
    uint maxParticles;
    float2 padding;
    float3 acceleration;
    float drag;
};

RWStructuredBuffer<Particle> particles : register(u0);
RWStructuredBuffer<uint> freeList : register(u1);
RWStructuredBuffer<int> freeListIndex : register(u2);

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint index = dispatchThreadId.x;
    if (index >= maxParticles) { return; }
    Particle particle = (Particle)0;
    particle.age = -1.0f;
    particles[index] = particle;
    freeList[index] = index;
    if (index == 0) { freeListIndex[0] = int(maxParticles) - 1; }
}
