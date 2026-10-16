#include "SrvManager.h"

#include <stdexcept>

#include "engine/core/DirectXCommon.h"

void SrvManager::Initialize(DirectXCommon *dxCommon) {
	// SRVヒープはDirectXCommonが生成済みである必要がある
	if (dxCommon == nullptr || dxCommon->GetSRVDescriptorHeap() == nullptr) {
		throw std::invalid_argument("SrvManager requires an initialized DirectXCommon instance.");
	}
	dxCommon_ = dxCommon;
	freeIndices_.clear();
	freeIndices_.reserve(kMaxSRVCount - 1);
	// 0番はImGuiが所有するため、ゲーム用アロケータから除外する。
	for (uint32_t index = kMaxSRVCount; index-- > 1;) {
		freeIndices_.push_back(index);
	}
}

SrvHandle SrvManager::Allocate() {
	if (dxCommon_ == nullptr) {
		throw std::logic_error("SrvManager is not initialized.");
	}
	if (freeIndices_.empty()) {
		throw std::runtime_error("The SRV descriptor heap is full.");
	}
	const uint32_t index = freeIndices_.back();
	freeIndices_.pop_back();
	Slot &slot = slots_[index];
	slot.allocated = true;
	return {index, slot.generation};
}

void SrvManager::Release(SrvHandle handle) {
	(void)Require(handle);
	Slot &slot = slots_[handle.index];
	slot.allocated = false;
	// 0は無効世代として予約しているため、オーバーフロー時は1へ戻す。
	if (++slot.generation == 0) {
		slot.generation = 1;
	}
	freeIndices_.push_back(handle.index);
}

bool SrvManager::IsValid(SrvHandle handle) const noexcept {
	return handle.IsValid() && handle.index < kMaxSRVCount && slots_[handle.index].allocated &&
	       slots_[handle.index].generation == handle.generation;
}

void SrvManager::CreateSRVforTexture2D(SrvHandle handle,
                                       ID3D12Resource *resource,
                                       const DirectX::TexMetadata &metadata) {
	if (dxCommon_ == nullptr || resource == nullptr) {
		throw std::invalid_argument("Invalid texture SRV creation request.");
	}
	(void)Require(handle);

	// 元画像の形式とミップ数を使って2Dテクスチャ用の設定を作る
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// 指定されたディスクリプタ位置へSRVを書き込む
	dxCommon_->GetDevice()->CreateShaderResourceView(resource, &srvDesc, GetCPUDescriptorHandle(handle));
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(SrvHandle handle) const {
	(void)Require(handle);
	return dxCommon_->GetCPUDescriptorHandleSRV(handle.index);
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(SrvHandle handle) const {
	(void)Require(handle);
	return dxCommon_->GetGPUDescriptorHandleSRV(handle.index);
}

const SrvManager::Slot &SrvManager::Require(SrvHandle handle) const {
	if (dxCommon_ == nullptr) {
		throw std::logic_error("SrvManager is not initialized.");
	}
	if (!IsValid(handle)) {
		throw std::invalid_argument("SRV handle is stale or not allocated.");
	}
	return slots_[handle.index];
}
