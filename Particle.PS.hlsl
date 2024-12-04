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

StructuredBuffer<Material> gMaterials : register(t1);
StructuredBuffer<DirectionalLight> gDirectionalLights : register(t2);

// MaterialとDirectionalLightをインデックスで参照する例
Material gMaterial = gMaterials[0];
DirectionalLight gDirectionalLight = gDirectionalLights[0];


Texture2D<float32_t4> gTexture : register(t0);

SamplerState gSampler : register(s0);



struct PixelShaderOutput
{
    float32_t4 color : SV_Target0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float32_t4 transformdUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformdUV.xy);
    
    output.color = gMaterial.color * textureColor;
    
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}