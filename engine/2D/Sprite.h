#pragma once

#include <cstdint>

#include <d3d12.h>
#include <wrl.h>

#include "engine/core/Mymath.h"
#include "engine/2d/SpriteCommon.h"

/// <summary>
/// 2Dスプライトを管理するクラス
/// </summary>
class Sprite {
public:
	/// <summary>
	/// スプライトに必要なGPUリソースを初期化する
	/// </summary>
	/// <param name="spriteCommon">スプライト描画共通処理</param>
	void Initialize(SpriteCommon* spriteCommon);

	/// <summary>
	/// 頂点バッファビューを取得する
	/// </summary>
	/// <returns>頂点バッファビュー</returns>
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;

	/// <summary>
	/// インデックスバッファビューを取得する
	/// </summary>
	/// <returns>インデックスバッファビュー</returns>
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

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
	/// マテリアルデータを取得する
	/// </summary>
	/// <returns>CPUから書き込むマテリアルデータ</returns>
	Material* GetMaterialData() const;

	/// <summary>
	/// 座標変換行列データを取得する
	/// </summary>
	/// <returns>CPUから書き込む座標変換行列データ</returns>
	TransformationMatrix* GetTransformationMatrixData() const;

private:
	SpriteCommon* spriteCommon_ = nullptr;  // スプライト描画共通処理

	// ===== GPUリソース =====
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;                // 頂点バッファ用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;                 // インデックスバッファ用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;              // マテリアル用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;  // 座標変換行列用リソース

	// ===== バッファビュー =====
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};  // 頂点バッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};    // インデックスバッファビュー

	// ===== CPU書き込み用データ =====
	Material* materialData_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;
};
