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
    Particle particle = particles[index];
    if (particle.age < 0.0f) { return; }
    particle.age += deltaTime;
    if (particle.age >= particle.lifetime)
    {
        particle.age = -1.0f;
        particles[index] = particle;
        int freeSlot;
        InterlockedAdd(freeListIndex[0], 1, freeSlot);
        freeList[freeSlot + 1] = index;
        return;
    }
    particle.velocity += acceleration * deltaTime;
    particle.velocity *= exp(-drag * deltaTime);
    particle.position += particle.velocity * deltaTime;
    particles[index] = particle;
}
