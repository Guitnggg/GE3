#include "ParticleCommon.hlsli"

cbuffer EmitConstants : register(b0)
{
    float3 emitPosition;
    uint emitCount;
    float3 minVelocity;
    uint seed;
    float3 maxVelocity;
    float minLifetime;
    float3 positionSpread;
    float maxLifetime;
    float4 startColor;
    float4 endColor;
    float2 startSize;
    float2 endSize;
};

RWStructuredBuffer<Particle> particles : register(u0);
RWStructuredBuffer<uint> freeList : register(u1);
RWStructuredBuffer<int> freeListIndex : register(u2);

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint threadIndex = dispatchThreadId.x;
    if (threadIndex >= emitCount) { return; }
    int oldFreeIndex;
    InterlockedAdd(freeListIndex[0], -1, oldFreeIndex);
    if (oldFreeIndex < 0)
    {
        InterlockedAdd(freeListIndex[0], 1);
        return;
    }

    const uint particleIndex = freeList[oldFreeIndex];
    const uint randomBase = seed + threadIndex * 747796405u;
    const float3 randomPosition = float3(Random01(randomBase), Random01(randomBase + 1), Random01(randomBase + 2)) * 2.0f - 1.0f;
    const float3 randomVelocity = float3(Random01(randomBase + 3), Random01(randomBase + 4), Random01(randomBase + 5));
    Particle particle;
    particle.position = emitPosition + randomPosition * positionSpread;
    particle.lifetime = lerp(minLifetime, maxLifetime, Random01(randomBase + 6));
    particle.velocity = lerp(minVelocity, maxVelocity, randomVelocity);
    particle.age = 0.0f;
    particle.startColor = startColor;
    particle.endColor = endColor;
    particle.startSize = startSize;
    particle.endSize = endSize;
    particles[particleIndex] = particle;
}
