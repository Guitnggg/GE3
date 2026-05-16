#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <vector>

#include "engine/base/Mymath.h"

/// <summary>
/// Object3dCommonクラスの前方宣言
/// </summary>
class Object3dCommon;

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
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

	/// <summary>
	/// 頂点バッファビューの取得
	/// </summary>
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;

	/// <summary>
	/// マテリアルリソースの取得
	/// </summary>
	ID3D12Resource* GetMaterialResource() const;

	/// <summary>
	/// 座標変換リソースの取得
	/// </summary>
	ID3D12Resource* GetTransformationMatrixResource() const;

	/// <summary>
	/// 平行光源リソースの取得
	/// </summary>
	ID3D12Resource* GetDirectionalLightResource() const;

	/// <summary>
	/// 座標変換データの取得
	/// </summary>
	TransformationMatrix* GetTransformationMatrixData() const;

	/// <summary>
	/// 平行光源データの取得
	/// </summary>
	DirectionalLight* GetDirectionalLightData() const;

	/// <summary>
	/// モデルデータの取得
	/// </summary>
	const ModelData& GetModelData() const;

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
