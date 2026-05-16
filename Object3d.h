#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <vector>

#include "Mymath.h"

/// <summary>
/// Object3dCommonクラスの前方宣言
/// </summary>
class Object3dCommon;

// 頂点データ
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

// マテリアルデータ
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

// 座標変換データ
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

	/// <summary>
	/// .mtlファイルの読み込み
	/// </summary>
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// .objファイルの読み込み
	/// </summary>
	static ModelData LoadObjectFile(const std::string& directoryPath, const std::string& filename);

private:
	Object3dCommon* object3dCommon_ = nullptr;  // Object3dCommonクラスのポインタ
	ModelData modelData_;  // objファイルのデータ

	// ===== 頂点データ =====
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	VertexData* vertexData_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// ===== マテリアル =====
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	// ===== 座標変換データ =====
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// ===== 平行光源 =====
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	DirectionalLight* directionalLightData_ = nullptr;

	// ===== 座標変換用のデータ =====
	Transform transform_{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Transform cameraTransform_{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.5f} };
};
