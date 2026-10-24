struct VertexShaderOutput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(uint vertexId : SV_VertexID) {
    // 頂点バッファを使わず、画面全体を覆う1枚の三角形を生成する。
    static const float2 positions[3] = {
        float2(-1.0f, 1.0f),
        float2(3.0f, 1.0f),
        float2(-1.0f, -3.0f),
    };

    static const float2 texcoords[3] = {
        float2(0.0f, 0.0f),
        float2(2.0f, 0.0f),
        float2(0.0f, 2.0f),
    };

    VertexShaderOutput output;
    output.position = float4(positions[vertexId], 0.0f, 1.0f);
    output.texcoord = texcoords[vertexId];
    return output;
}
