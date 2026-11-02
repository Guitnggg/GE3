Texture2D<float4> sourceTexture : register(t0);
SamplerState sourceSampler : register(s0);

cbuffer RadialBlurParameters : register(b0) {
    float2 center;
    float blurWidth;
    uint sampleCount;
};

static const uint kMaxSampleCount = 32;

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD0) : SV_TARGET {
    // 現在Pixelから中心へ向かうVectorを求め、その一部をSampling範囲として使用する。
    float2 directionToCenter = center - texcoord;
    float4 accumulatedColor = 0.0f;

    // CPU側でも2～32へ制限するが、Shader単体利用時の安全性も確保する。
    uint safeSampleCount = clamp(sampleCount, 2u, kMaxSampleCount);

    [loop]
    for (uint index = 0; index < safeSampleCount; ++index) {
        // 0では元Pixel、1ではblurWidth分だけ中心へ進んだ位置をSamplingする。
        float ratio = float(index) / float(safeSampleCount - 1u);
        float2 sampleUv = texcoord + directionToCenter * blurWidth * ratio;
        accumulatedColor += sourceTexture.Sample(sourceSampler, sampleUv);
    }

    // Sampling数で平均化し、Effect有効時に画面全体の明るさが変化することを防ぐ。
    return accumulatedColor / float(safeSampleCount);
}
