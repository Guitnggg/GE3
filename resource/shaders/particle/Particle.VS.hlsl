#include "ParticleCommon.hlsli"

cbuffer DrawConstants : register(b0)
{
    float4x4 viewProjection;
    float3 cameraRight;
    float padding0;
    float3 cameraUp;
    float padding1;
};

StructuredBuffer<Particle> particles : register(t0);

struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color : COLOR0;
};

VertexOutput main(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
    static const float2 corners[6] = {
        float2(-0.5f,  0.5f), float2(0.5f,  0.5f), float2(-0.5f, -0.5f),
        float2(-0.5f, -0.5f), float2(0.5f,  0.5f), float2(0.5f, -0.5f)
    };
    static const float2 uvs[6] = {
        float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f),
        float2(0.0f, 1.0f), float2(1.0f, 0.0f), float2(1.0f, 1.0f)
    };
    const Particle particle = particles[instanceId];
    VertexOutput output;
    if (particle.age < 0.0f || particle.lifetime <= 0.0f)
    {
        output.position = float4(2.0f, 2.0f, 2.0f, 1.0f);
        output.texcoord = 0.0f;
        output.color = 0.0f;
        return output;
    }
    const float progress = saturate(particle.age / particle.lifetime);
    const float2 size = lerp(particle.startSize, particle.endSize, progress);
    const float2 corner = corners[vertexId] * size;
    const float3 worldPosition = particle.position + cameraRight * corner.x + cameraUp * corner.y;
    output.position = mul(float4(worldPosition, 1.0f), viewProjection);
    output.texcoord = uvs[vertexId];
    output.color = lerp(particle.startColor, particle.endColor, progress);
    return output;
}
