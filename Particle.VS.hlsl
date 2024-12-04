#include "Particle.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
};
StructuredBuffer<TransformationMatrix> gTransformationMatrices : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    // instanceIdを使用して適切なTransformationMatrixを取得
    TransformationMatrix gTranfsformationMatrix = gTransformationMatrices[instanceId];

    VertexShaderOutput output;
    output.position = mul(input.position, gTranfsformationMatrix.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) gTranfsformationMatrix.World));
    return output;
}
