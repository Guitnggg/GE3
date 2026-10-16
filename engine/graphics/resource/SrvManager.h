#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <vector>

#include <d3d12.h>

#include "externals/DirectXTex/DirectXTex.h"

class DirectXCommon;

/// <summary>
/// 再利用されたSRVスロットを古い参照から区別する世代付きハンドル。
/// </summary>
struct SrvHandle {
	static constexpr uint32_t kInvalidIndex = std::numeric_limits<uint32_t>::max();
	uint32_t index = kInvalidIndex;
	uint32_t generation = 0;

	[[nodiscard]] bool IsValid() const noexcept {
		return index != kInvalidIndex && generation != 0;
	}
	bool operator==(const SrvHandle &) const noexcept = default;
};

/// <summary>
/// SRVヒープの番号割り当てとCPU・GPUディスクリプタ参照を管理する。
/// </summary>
class SrvManager {
  public:
	// 作成可能なSRVの最大数
	static const uint32_t kMaxSRVCount = 128;

	/// <summary>
	/// DirectX共通処理を受け取り、SRV管理を初期化する。
	/// </summary>
	void Initialize(DirectXCommon *dxCommon);

	/// <summary>
	/// 未使用のSRVスロットを1つ割り当て、現在世代のハンドルを返す。
	/// </summary>
	[[nodiscard]] SrvHandle Allocate();

	/// <summary>
	/// 使用済みSRVスロットを解放し、次回の割り当て候補へ戻す。
	/// </summary>
	void Release(SrvHandle handle);

	/// <summary>
	/// ハンドルが現在割り当て中のSRVを指しているか返す。
	/// </summary>
	[[nodiscard]] bool IsValid(SrvHandle handle) const noexcept;

	/// <summary>
	/// 2Dテクスチャ用のSRVを指定位置に作成する。
	/// </summary>
	void CreateSRVforTexture2D(SrvHandle handle, ID3D12Resource *resource, const DirectX::TexMetadata &metadata);

	/// <summary>
	/// 有効なSRVハンドルに対応するCPUディスクリプタを取得する。
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(SrvHandle handle) const;

	/// <summary>
	/// 有効なSRVハンドルに対応するGPUディスクリプタを取得する。
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(SrvHandle handle) const;

  private:
	struct Slot {
		uint32_t generation = 1; // 解放のたびに進み、古いハンドルとの一致を防ぐ
		bool allocated = false;  // 現在ディスクリプタの所有者がいるか
	};

	[[nodiscard]] const Slot &Require(SrvHandle handle) const;

	DirectXCommon *dxCommon_ = nullptr;      // DirectX共通処理
	std::array<Slot, kMaxSRVCount> slots_{}; // SRV番号ごとの世代と割り当て状態
	std::vector<uint32_t> freeIndices_;      // 未使用番号を末尾から定数時間で取得するリスト
};
