#pragma once

#include <cstdint>

#include <d3d12.h>

#include "externals/DirectXTex/DirectXTex.h"

class DirectXCommon;

/// <summary>
/// SRVディスクリプタの割り当てと、CPU・GPUハンドルの取得を管理するクラス。
/// </summary>
class SrvManager {
public:
	// 作成可能なSRVの最大数
	static const uint32_t kMaxSRVCount = 128;

	/// <summary>
	/// DirectX共通処理を受け取り、SRV管理を初期化する。
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 未使用のSRV番号を1つ割り当てる。
	/// </summary>
	uint32_t Allocate();

	/// <summary>
	/// 2Dテクスチャ用のSRVを指定位置に作成する。
	/// </summary>
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* resource, const DirectX::TexMetadata& metadata);

	/// <summary>
	/// 指定番号のCPUディスクリプタハンドルを取得する。
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index) const;
	
	/// <summary>
	/// 指定番号のGPUディスクリプタハンドルを取得する。
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index) const;
	
private:
	DirectXCommon* dxCommon_ = nullptr; // DirectX共通処理
	uint32_t useIndex_ = 1;             // 次に割り当てるSRV番号（0番はImGui用）
};
