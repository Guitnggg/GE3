#pragma once

#include "engine/math/MathTypes.h"

#include <cstdint>
#include <string>

/// <summary>
/// 頂点シェーダーへ入力する位置、UV、法線の組。
/// </summary>
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

/// <summary>
/// ピクセルシェーダーへ渡す表面色とUV変換情報。
/// </summary>
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3]; // 定数バッファのアライメント調整用
	Matrix4x4 uvTransform;
};

/// <summary>
/// 外部マテリアルから取得したテクスチャ参照情報。
/// </summary>
struct MaterialData {
	std::string textureFilePath;
};

/// <summary>
/// オブジェクト描画で使用するワールド行列と合成行列。
/// </summary>
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

/// <summary>
/// 平行光源の色、向き、明るさをまとめた定数データ。
/// </summary>
struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};
