#pragma once

#include <cstdint>

#include <d3d12.h>
#include <wrl.h>

#include "Mymath.h"
#include "SpriteCommon.h"

/// <summary>
/// 
/// </summary>
class Sprite {
public:
	/// <summary>
	/// 
	/// </summary>
	/// <param name="spriteCommon"></param>
	void Initialize(SpriteCommon* spriteCommon);

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	ID3D12Resource* GetMaterialResource() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	ID3D12Resource* GetTransformationMatrixResource() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	Material* GetMaterialData() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	TransformationMatrix* GetTransformationMatrixData() const;

private:
	// 
	SpriteCommon* spriteCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;                // 頂点バッファー用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;                 // インデックスバッファー用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;              // マテリアル用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;  // ワールド行列用のリソース

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};  // 頂点バッファービュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};    // インデックスバッファービュー

	// マテリアルデータ
	Material* materialData_ = nullptr;

	// ワールド行列データ
	TransformationMatrix* transformationMatrixData_ = nullptr;
};