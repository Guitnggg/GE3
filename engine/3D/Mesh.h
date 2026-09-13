#pragma once

#include <cstdint>
#include <d3d12.h>
#include <wrl.h>
#include <vector>

#include "engine/core/Mymath.h"

class DirectXCommon;

/// <summary>
/// 頂点データをGPUへ転送し、頂点バッファと描画情報を管理するクラス。
/// Modelから所有され、複数のObject3dから共有して使用される。
/// </summary>
class Mesh {
public:
	/// <summary>
	/// 頂点列からGPU頂点バッファを生成する。
	/// </summary>
	/// <param name="dxCommon">GPUリソースの生成に使用するDirectX共通処理</param>
	/// <param name="vertices">三角形リスト形式の頂点データ</param>
	void Initialize(DirectXCommon* dxCommon, const std::vector<VertexData>& vertices);

	/// <summary>
	/// 頂点バッファを設定し、メッシュを1インスタンス描画する。
	/// 呼び出し前にルートシグネチャや定数バッファを設定しておく必要がある。
	/// </summary>
	/// <param name="commandList">描画命令を記録するコマンドリスト</param>
	void Draw(ID3D12GraphicsCommandList* commandList) const;

	/// <summary>GPU頂点バッファが生成済みかを取得する。</summary>
	/// <returns>初期化済みの場合はtrue</returns>
	bool IsInitialized() const { return vertexResource_ != nullptr; }

	/// <summary>描画に使用する頂点数を取得する。</summary>
	/// <returns>頂点数</returns>
	uint32_t GetVertexCount() const { return vertexCount_; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_; // GPU上の頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};           // 描画時に使用する頂点バッファ情報
	uint32_t vertexCount_ = 0;                              // DrawInstancedへ渡す頂点数
};
