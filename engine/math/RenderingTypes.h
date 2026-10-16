#pragma once

#include "engine/math/MathTypes.h"

#include <cstdint>
#include <string>

/// <summary>
/// 頂点シェーダーへ入力する位置、UV、法線の組。
/// </summary>
struct VertexData {
	Vector4 position; // オブジェクト空間上の同次座標
	Vector2 texcoord; // テクスチャを参照するUV座標
	Vector3 normal;   // ライティングに使用する頂点法線
};

/// <summary>
/// ピクセルシェーダーへ渡す表面色とUV変換情報。
/// </summary>
struct Material {
	Vector4 color;          // テクスチャ色へ乗算する基本色
	int32_t enableLighting; // 平行光源によるライティングを有効にするか
	float padding[3];       // 定数バッファのアライメント調整用
	Matrix4x4 uvTransform;  // UV座標へ適用する変換行列
};

/// <summary>
/// 外部マテリアルから取得したテクスチャ参照情報。
/// </summary>
struct MaterialData {
	std::string textureFilePath; // マテリアルが参照するテクスチャファイルのパス
};

/// <summary>
/// オブジェクト描画で使用するワールド行列と合成行列。
/// </summary>
struct TransformationMatrix {
	Matrix4x4 WVP;   // ローカル座標をクリップ座標へ変換する合成行列
	Matrix4x4 World; // ローカル座標をワールド座標へ変換する行列
};

/// <summary>
/// 平行光源の色、向き、明るさをまとめた定数データ。
/// </summary>
struct DirectionalLight {
	Vector4 color;     // 光源の色
	Vector3 direction; // 光が進む正規化済み方向
	float intensity;   // 光の明るさ倍率
};
