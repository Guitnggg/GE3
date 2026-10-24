Texture2D<float4> sourceTexture : register(t0);
SamplerState sourceSampler : register(s0);

cbuffer VignettingParameters : register(b0) {
    float intensity;
    float power;
    float2 padding;
};

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD0) : SV_TARGET {
    float4 color = sourceTexture.Sample(sourceSampler, texcoord);

    // UVと反転UVを掛けると、中央で最大、四隅で0になる対称的な係数が得られる。
    float2 distanceFromEdge = texcoord * (1.0f - texcoord.yx);
    float vignette = distanceFromEdge.x * distanceFromEdge.y * intensity;

    // saturateで0～1へ収め、powerで中央から端へ暗くなるカーブを調整する。
    vignette = saturate(pow(saturate(vignette), power));
    color.rgb *= vignette;

    // AlphaはUI合成など後段の処理へ影響させないため、入力値を維持する。
    return color;
}
