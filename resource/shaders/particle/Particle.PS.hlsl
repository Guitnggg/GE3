Texture2D<float4> particleTexture : register(t1);
SamplerState particleSampler : register(s0);

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color : COLOR0;
};

float4 main(PixelInput input) : SV_TARGET
{
    const float4 color = particleTexture.Sample(particleSampler, input.texcoord) * input.color;
    if (color.a <= 0.001f) { discard; }
    return color;
}
