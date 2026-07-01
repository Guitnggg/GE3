#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <vector>

#include "engine/core/Mymath.h"

/// <summary>
/// Object3dCommonクラスの前方宣言
/// </summary>
class Object3dCommon;

/// <summary>
/// OBJモデルから読み込んだ頂点情報とマテリアル情報
/// </summary>
struct ModelData {
	std::vector<VertexData> vertices;  // モデルを構成する頂点データ
	MaterialData material;             // モデルに紐づくマテリアルデータ
};

/// <summary>
/// 3Dオブジェクトを管理するクラス
/// </summary>
class Object3d {
public:
	/// <summary>
	/// 3Dオブジェクトに必要なGPUリソースを初期化する
	/// </summary>
	/// <param name="object3dCommon">3D描画共通処理</param>
	void Initialize(Object3dCommon* object3dCommon);

	/// <summary>
	/// .mtlファイルを読み込む
	/// </summary>
	/// <param name="directoryPath">ファイルがあるディレクトリ</param>
	/// <param name="filename">読み込む.mtlファイル名</param>
	/// <returns>マテリアルデータ</returns>
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// .objファイルを読み込む
	/// </summary>
	/// <param name="directoryPath">ファイルがあるディレクトリ</param>
	/// <param name="filename">読み込む.objファイル名</param>
	/// <returns>モデルデータ</returns>
	static ModelData LoadObjectFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// 頂点バッファビューを取得する
	/// </summary>
	/// <returns>頂点バッファビュー</returns>
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;

	/// <summary>
	/// マテリアルリソースを取得する
	/// </summary>
	/// <returns>マテリアル用GPUリソース</returns>
	ID3D12Resource* GetMaterialResource() const;

	/// <summary>
	/// 座標変換行列リソースを取得する
	/// </summary>
	/// <returns>座標変換行列用GPUリソース</returns>
	ID3D12Resource* GetTransformationMatrixResource() const;

	/// <summary>
	/// 平行光源リソースを取得する
	/// </summary>
	/// <returns>平行光源用GPUリソース</returns>
	ID3D12Resource* GetDirectionalLightResource() const;

	/// <summary>
	/// 座標変換行列データを取得する
	/// </summary>
	/// <returns>CPUから書き込む座標変換行列データ</returns>
	TransformationMatrix* GetTransformationMatrixData() const;

	/// <summary>
	/// 平行光源データを取得する
	/// </summary>
	/// <returns>CPUから書き込む平行光源データ</returns>
	DirectionalLight* GetDirectionalLightData() const;

	/// <summary>
	/// モデルデータを取得する
	/// </summary>
	/// <returns>読み込み済みモデルデータ</returns>
	const ModelData& GetModelData() const;

private:
	Object3dCommon* object3dCommon_ = nullptr;  // 3D描画共通処理
	ModelData modelData_;                       // objファイルから読み込んだモデルデータ

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

	// ===== 座標変換用データ =====
	Transform transform_{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Transform cameraTransform_{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.5f} };
};
