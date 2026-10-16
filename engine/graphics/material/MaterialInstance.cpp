#include "engine/graphics/material/MaterialInstance.h"

#include "engine/core/DirectXCommon.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/math/MatrixMath.h"

#include <cstring>
#include <stdexcept>

void MaterialInstance::Initialize(DirectXCommon *dxCommon, TextureHandle textureHandle) {
	if (dxCommon == nullptr || dxCommon->GetDevice() == nullptr) {
		throw std::invalid_argument("MaterialInstance requires initialized DirectXCommon.");
	}
	if (IsInitialized()) {
		throw std::logic_error("MaterialInstance is already initialized.");
	}

	dxCommon_ = dxCommon;
	data_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	data_.enableLighting = true;
	data_.uvTransform = MakeIdentity4x4();
	textureHandle_ = textureHandle;
	blendMode_ = BlendMode::Opaque;
}

void MaterialInstance::Bind(ID3D12GraphicsCommandList *commandList, const TextureManager &textureManager) const {
	if (!IsInitialized() || commandList == nullptr) {
		throw std::logic_error("MaterialInstance cannot bind before initialization.");
	}
	// Bind時点のFrameContextへコピーし、前フレームを読むGPUとの書き込み競合を避ける。
	const auto allocation = dxCommon_->AllocateDynamicBuffer(sizeof(data_));
	std::memcpy(allocation.cpuAddress, &data_, sizeof(data_));
	commandList->SetGraphicsRootConstantBufferView(0, allocation.gpuAddress);
	commandList->SetGraphicsRootDescriptorTable(2, textureManager.GetSrvHandleGPU(textureHandle_));
}

Material &MaterialInstance::GetData() {
	if (!IsInitialized()) {
		throw std::logic_error("MaterialInstance is not initialized.");
	}
	return data_;
}

const Material &MaterialInstance::GetData() const {
	if (!IsInitialized()) {
		throw std::logic_error("MaterialInstance is not initialized.");
	}
	return data_;
}

void MaterialInstance::SetBlendMode(BlendMode blendMode) {
	if (blendMode >= BlendMode::Count) {
		throw std::invalid_argument("Invalid material blend mode.");
	}
	blendMode_ = blendMode;
}
