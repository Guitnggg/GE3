#include "TextureManager.h"

#include <stdexcept>

#include "engine/graphics/resource/SrvManager.h"
#include "engine/core/DirectXCommon.h"

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	// テクスチャ生成とSRV作成に必要な管理クラスを保持する
	if (dxCommon == nullptr || srvManager == nullptr) {
		throw std::invalid_argument("TextureManager requires DirectXCommon and SrvManager.");
	}
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

uint32_t TextureManager::Load(const std::string& filePath) {
	if (dxCommon_ == nullptr || srvManager_ == nullptr) {
		throw std::logic_error("TextureManager is not initialized.");
	}

	// 読み込み済みなら新しいGPUリソースを作らず、既存の番号を返す
	if (textureIndexMap_.contains(filePath)) {
		return textureIndexMap_[filePath];
	}

	// 画像を読み込み、ミップマップを含むテクスチャリソースをGPUへ転送する
	DirectX::ScratchImage mipImages = dxCommon_->LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	TextureData textureData{};
	textureData.metadata = metadata;
	textureData.resource = dxCommon_->CreateTextureResource(dxCommon_->GetDevice(), metadata);
	dxCommon_->UploadTextureData(textureData.resource, mipImages);
	// 空いているディスクリプタを確保し、シェーダーから参照できるSRVを作る
	textureData.srvIndex = srvManager_->Allocate();
	srvManager_->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(), textureData.metadata);

	// 次回以降に同じファイルを再利用できるよう、配列と検索表へ登録する
	uint32_t textureIndex = static_cast<uint32_t>(textures_.size());
	textures_.push_back(textureData);
	textureIndexMap_[filePath] = textureIndex;
	return textureIndex;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex) const {
	// 無効なテクスチャ番号からGPUハンドルを取得しないよう検証する
	if (srvManager_ == nullptr) {
		throw std::logic_error("TextureManager is not initialized.");
	}
	if (textureIndex >= textures_.size()) {
		throw std::out_of_range("Invalid texture index.");
	}
	return srvManager_->GetGPUDescriptorHandle(textures_[textureIndex].srvIndex);
}
