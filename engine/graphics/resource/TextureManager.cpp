#include "TextureManager.h"

#include <algorithm>
#include <stdexcept>

#include "engine/core/DirectXCommon.h"

void TextureManager::Initialize(DirectXCommon *dxCommon, SrvManager *srvManager) {
	// テクスチャ生成とSRV作成に必要な管理クラスを保持する
	if (dxCommon == nullptr || srvManager == nullptr) {
		throw std::invalid_argument("TextureManager requires DirectXCommon and SrvManager.");
	}
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

TextureHandle TextureManager::Load(const std::string &filePath) {
	if (dxCommon_ == nullptr || srvManager_ == nullptr) {
		throw std::logic_error("TextureManager is not initialized.");
	}
	CollectGarbage();

	// 同じパスが有効な間はGPUリソースとハンドルを共有する。
	if (const auto found = textureHandleMap_.find(filePath); found != textureHandleMap_.end()) {
		++textureSlots_[found->second.index].data->referenceCount;
		return found->second;
	}

	// CPUデコードとGPU転送を別工程にし、前者だけをワーカースレッドへ移せる境界を保つ。
	TextureData textureData = Upload(Decode(filePath));

	uint32_t index = 0;
	if (freeTextureIndices_.empty()) {
		index = static_cast<uint32_t>(textureSlots_.size());
		textureSlots_.push_back({});
	} else {
		index = freeTextureIndices_.back();
		freeTextureIndices_.pop_back();
	}
	TextureSlot &slot = textureSlots_[index];
	slot.data = std::move(textureData);
	const TextureHandle handle{index, slot.generation};
	textureHandleMap_.emplace(filePath, handle);
	return handle;
}

void TextureManager::Unload(TextureHandle handle) {
	(void)Require(handle);
	TextureSlot &slot = textureSlots_[handle.index];
	if (slot.data->referenceCount > 1) {
		// 同じパスをLoadした別の利用者が残っている間は、共有ハンドルと実体を維持する。
		--slot.data->referenceCount;
		return;
	}
	TextureData texture = std::move(*slot.data);
	textureHandleMap_.erase(texture.path);

	// ハンドルは即時無効化するが、GPUが参照し終えるまで実体とSRVスロットは保持する。
	pendingReleases_.push_back(
	    {std::move(texture.resource), texture.srvHandle, dxCommon_->GetSafeReleaseFenceValue()});
	slot.data.reset();
	if (++slot.generation == 0) {
		slot.generation = 1;
	}
	freeTextureIndices_.push_back(handle.index);
	CollectGarbage();
}

void TextureManager::CollectGarbage() {
	if (dxCommon_ == nullptr || srvManager_ == nullptr) {
		return;
	}
	const uint64_t completedFence = dxCommon_->GetCompletedFenceValue();
	std::erase_if(pendingReleases_, [this, completedFence](PendingRelease &release) {
		if (release.fenceValue > completedFence) {
			return false;
		}
		// リソースを捨てる直前にSRVを返し、同じスロットが早期に上書きされることを防ぐ。
		srvManager_->Release(release.srvHandle);
		return true;
	});
}

bool TextureManager::IsValid(TextureHandle handle) const noexcept {
	return handle.IsValid() && handle.index < textureSlots_.size() &&
	       textureSlots_[handle.index].generation == handle.generation && textureSlots_[handle.index].data.has_value();
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(TextureHandle handle) const {
	return srvManager_->GetGPUDescriptorHandle(Require(handle).srvHandle);
}

const TextureManager::TextureData &TextureManager::Require(TextureHandle handle) const {
	if (dxCommon_ == nullptr || srvManager_ == nullptr) {
		throw std::logic_error("TextureManager is not initialized.");
	}
	if (!IsValid(handle)) {
		throw std::invalid_argument("Texture handle is stale or unloaded.");
	}
	return *textureSlots_[handle.index].data;
}

TextureManager::DecodedTexture TextureManager::Decode(const std::string &filePath) const {
	// この工程ではGPU状態へ触れない。将来の非同期ロードではジョブキュー側で実行する。
	return {dxCommon_->LoadTexture(filePath), filePath};
}

TextureManager::TextureData TextureManager::Upload(DecodedTexture decodedTexture) {
	TextureData textureData{};
	textureData.metadata = decodedTexture.mipImages.GetMetadata();
	textureData.path = std::move(decodedTexture.path);
	textureData.resource = dxCommon_->CreateTextureResource(dxCommon_->GetDevice(), textureData.metadata);
	dxCommon_->UploadTextureData(textureData.resource, decodedTexture.mipImages);
	textureData.srvHandle = srvManager_->Allocate();
	try {
		srvManager_->CreateSRVforTexture2D(textureData.srvHandle, textureData.resource.Get(), textureData.metadata);
	} catch (...) {
		// SRV作成前に失敗しても、確保済みスロットを失わないよう割り当てを戻す。
		srvManager_->Release(textureData.srvHandle);
		throw;
	}
	return textureData;
}
