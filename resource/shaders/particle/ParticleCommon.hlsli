struct Particle
{
    float3 position;
    float lifetime;
    float3 velocity;
    float age;
    float4 startColor;
    float4 endColor;
    float2 startSize;
    float2 endSize;
};

float Random01(uint value)
{
    value ^= value >> 16;
    value *= 0x7feb352d;
    value ^= value >> 15;
    value *= 0x846ca68b;
    value ^= value >> 16;
    return (value & 0x00ffffff) / 16777215.0f;
}
