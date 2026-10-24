Texture2D<float4> sourceTexture : register(t0);
SamplerState sourceSampler : register(s0);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD0) : SV_TARGET {
    // 現段階では色を変更せずコピーする。後続工程のポストエフェクト挿入点となる。
    return sourceTexture.Sample(sourceSampler, texcoord);
}
