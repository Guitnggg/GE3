#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <vector>

#include "Mesh.h"
#include "engine/core/Mymath.h"

/// <summary>
/// Object3dCommonクラスの前方宣言
/// </summary>
class Object3dCommon;
class TextureManager;

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
	void Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
		const std::string& directoryPath = "resource",
		const std::string& filename = "axis.obj");
	void Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
		const std::vector<VertexData>& vertices, uint32_t textureIndex);

	/// <summary>
	/// モデル固有のリソースとテクスチャを設定して描画する
	/// </summary>
	void Draw() const;

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
	/// 座標変換行列データを取得する
	/// </summary>
	/// <returns>CPUから書き込む座標変換行列データ</returns>
	TransformationMatrix* GetTransformationMatrixData() const;

	/// <summary>
	/// 平行光源データを取得する
	/// </summary>
	/// <returns>CPUから書き込む平行光源データ</returns>
	DirectionalLight* GetDirectionalLightData() const;
	Material* GetMaterialData() const;
	void SetTextureIndex(uint32_t textureIndex);

private:
	void InitializeResources(const std::vector<VertexData>& vertices);
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU() const;

	Object3dCommon* object3dCommon_ = nullptr;  // 3D描画共通処理
	TextureManager* textureManager_ = nullptr;  // モデルのテクスチャ管理
	uint32_t textureIndex_ = 0;                  // TextureManager内のテクスチャ番号

	Mesh mesh_;

	// ===== マテリアル =====
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	// ===== 座標変換データ =====
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// ===== 平行光源 =====
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	DirectionalLight* directionalLightData_ = nullptr;

};
