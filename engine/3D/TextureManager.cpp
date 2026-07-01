#include "TextureManager.h"

#include <cassert>

#include "engine/3D/SrvManager.h"
#include "engine/core/DirectXCommon.h"

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	assert(dxCommon != nullptr);
	assert(srvManager != nullptr);
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

uint32_t TextureManager::Load(const std::string& filePath) {
	assert(dxCommon_ != nullptr);
	assert(srvManager_ != nullptr);

	if (textureIndexMap_.contains(filePath)) {
		return textureIndexMap_[filePath];
	}

	DirectX::ScratchImage mipImages = dxCommon_->LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	TextureData textureData{};
	textureData.metadata = metadata;
	textureData.resource = dxCommon_->CrateTextureResource(dxCommon_->GetDevice(), metadata);
	dxCommon_->UploadTextureData(textureData.resource, mipImages);
	textureData.srvIndex = srvManager_->Allocate();
	srvManager_->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(), textureData.metadata);

	uint32_t textureIndex = static_cast<uint32_t>(textures_.size());
	textures_.push_back(textureData);
	textureIndexMap_[filePath] = textureIndex;
	return textureIndex;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex) const {
	assert(srvManager_ != nullptr);
	assert(textureIndex < textures_.size());
	return srvManager_->GetGPUDescriptorHandle(textures_[textureIndex].srvIndex);
}
