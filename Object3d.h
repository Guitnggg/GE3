#pragma once

#include <string>

#include "Mymath.h"

/// <summary>
/// Object3dCommonクラスの前方宣言 
/// </summary>
class Object3dCommon;

struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

// 頂点データ
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

// マテリアルデータ
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

// 座標返還データ
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

// 平行光源データ
struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

/// <summary>
/// 3Dオブジェクトクラス
/// </summary>
class Object3d {
public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="object3dCommon"></param>
	void Initialize(Object3dCommon* object3dCommon);

	// objファイルのデータ
	ModelData modelData;

	// Object3dCommonクラスのポインタ
	Object3dCommon* object3dCommon_ = nullptr;

	/// <summary>
	///  .mtlファイルの読み込み
	/// </summary>
	/// <param name="directoryPath"></param>
	/// <param name="filename"></param>
	/// <returns></returns>
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// .objファイルの読み込み
	/// </summary>
	/// <param name="directoryPath"></param>
	/// <param name="filename"></param>
	/// <returns></returns>
	static ModelData LoadObjectFile(const std::string& directoryPath, const std::string& filename);


	頂点データ
	// バッファリソース
	VertexResource (VertexBuffer)

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_DATA_VIEW vertexBufferView{};


	マテリアル
	// バッファリソース
	マテリアルリソース(ConstantBuffer)

	// バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;


	座標変換行列
	// バッファリソース
	座標変換行列リソース(ConstantBuffer)

	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;


	平行光源
	// バッファリソース
	平行光源リソース(ConstantBuffer)

	// バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData = nullptr;


	Transform transform;
	Transform cameraTransform;
};
