#pragma once

#include "engine/math/MathTypes.h"

#include <cstdint>
#include <string>

/// <summary>
/// 頂点シェーダーへ渡す頂点データ
/// </summary>
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

/// <summary>
/// 描画用マテリアルデータ
/// </summary>
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3]; // 定数バッファのアライメント調整用
	Matrix4x4 uvTransform;
};

/// <summary>
/// マテリアルファイルから読み込んだデータ
/// </summary>
struct MaterialData {
	std::string textureFilePath;
};

/// <summary>
/// シェーダーへ渡す座標変換行列
/// </summary>
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

/// <summary>
/// 平行光源データ
/// </summary>
struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};
