#pragma once

#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <wrl.h>

#include "engine/math/Mymath.h"

class Camera;
class Model;
class Object3dCommon;
class TextureManager;

/// <summary>
/// シーン上に配置する3Dオブジェクトを管理するクラス。
/// Modelのメッシュは共有し、配置ごとのマテリアル、座標変換、ライトを個別に所有する。
/// </summary>
class Object3d {
public:
	/// <summary>共有モデルと描画管理クラスを受け取り、オブジェクト固有のGPUリソースを生成する。</summary>
	/// <param name="object3dCommon">3D描画パイプラインを管理する共通処理</param>
	/// <param name="textureManager">描画テクスチャのSRVを取得する管理クラス</param>
	/// <param name="model">このオブジェクトが描画する共有モデル</param>
	void Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
		const std::shared_ptr<Model>& model);

	/// <summary>
	/// 現在のTransformとカメラから描画用行列を計算し、GPU定数バッファを更新する。
	/// </summary>
	/// <param name="camera">更新済みのビュー・プロジェクション行列を持つカメラ</param>
	void Update(const Camera& camera);

	/// <summary>オブジェクト固有データを設定し、共有モデルを描画する。</summary>
	void Draw() const;

	/// <summary>位置・回転・拡縮を変更するためのTransformを取得する。</summary>
	/// <returns>このオブジェクトが所有するTransform</returns>
	Transform& GetTransform() { return transform_; }

	/// <summary>位置・回転・拡縮を読み取るためのTransformを取得する。</summary>
	/// <returns>このオブジェクトが所有するTransform</returns>
	const Transform& GetTransform() const { return transform_; }

	/// <summary>CPUから更新可能な平行光源情報を取得する。</summary>
	/// <returns>GPU定数バッファへマップされた平行光源データ</returns>
	DirectionalLight* GetDirectionalLightData() const { return directionalLightData_; }

	/// <summary>CPUから更新可能なマテリアル情報を取得する。</summary>
	/// <returns>GPU定数バッファへマップされたマテリアルデータ</returns>
	Material* GetMaterialData() const { return materialData_; }

	/// <summary>このオブジェクトの描画に使用するテクスチャを変更する。</summary>
	/// <param name="textureIndex">TextureManagerが発行したテクスチャ番号</param>
	void SetTextureIndex(uint32_t textureIndex) { textureIndex_ = textureIndex; }

private:
	Object3dCommon* object3dCommon_ = nullptr; // 3D描画パイプラインの参照
	TextureManager* textureManager_ = nullptr; // テクスチャSRV管理の参照
	std::shared_ptr<Model> model_;             // 複数オブジェクト間で共有するモデル資産
	uint32_t textureIndex_ = 0;                // この配置で使用するテクスチャ番号
	Transform transform_{{1.0f, 1.0f, 1.0f}, {}, {}}; // シーン上の拡縮・回転・位置
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_; // マテリアル定数バッファ
	Material* materialData_ = nullptr;                          // マップ済みマテリアル書き込み先
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_; // 行列定数バッファ
	TransformationMatrix* transformationMatrixData_ = nullptr;             // マップ済み行列書き込み先
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_; // 平行光源定数バッファ
	DirectionalLight* directionalLightData_ = nullptr;                // マップ済みライト書き込み先
};
