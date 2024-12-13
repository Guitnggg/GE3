#include"Particle.hlsli"


//float4 main() : SV_TARGET
//{
//    return float4(1.0f, 1.0f, 1.0f, 1.0f);
//}

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
};

struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

ConstantBuffer<Material> gMaterials : register(b1);
ConstantBuffer<DirectionalLight> gDirectionalLights : register(b2);


Texture2D<float32_t4> gTexture : register(t0);

SamplerState gSampler : register(s0);



struct PixelShaderOutput
{
    float32_t4 color : SV_Target0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float32_t4 transformdUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterials.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformdUV.xy);
    
    output.color = gMaterials.color * textureColor * input.color;
    
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}