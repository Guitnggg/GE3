#include "engine/graphics/material/MaterialInstance.h"

#include "engine/core/DirectXCommon.h"
#include "engine/core/diagnostics/HResult.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/math/MatrixMath.h"

#include <stdexcept>

void MaterialInstance::Initialize(DirectXCommon *dxCommon, uint32_t textureIndex) {
	if (dxCommon == nullptr || dxCommon->GetDevice() == nullptr) {
		throw std::invalid_argument("MaterialInstance requires initialized DirectXCommon.");
	}
	if (IsInitialized()) {
		throw std::logic_error("MaterialInstance is already initialized.");
	}

	resource_ = dxCommon->CreateBufferResource(sizeof(Material));
	HResult::ThrowIfFailed(resource_->Map(0, nullptr, reinterpret_cast<void **>(&data_)),
	                       "Mapping the material constant buffer");
	data_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	data_->enableLighting = true;
	data_->uvTransform = MakeIdentity4x4();
	textureIndex_ = textureIndex;
	blendMode_ = BlendMode::Opaque;
}

void MaterialInstance::Bind(ID3D12GraphicsCommandList *commandList, const TextureManager &textureManager) const {
	if (!IsInitialized() || commandList == nullptr) {
		throw std::logic_error("MaterialInstance cannot bind before initialization.");
	}
	commandList->SetGraphicsRootConstantBufferView(0, resource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager.GetSrvHandleGPU(textureIndex_));
}

Material &MaterialInstance::GetData() {
	if (!IsInitialized()) {
		throw std::logic_error("MaterialInstance is not initialized.");
	}
	return *data_;
}

const Material &MaterialInstance::GetData() const {
	if (!IsInitialized()) {
		throw std::logic_error("MaterialInstance is not initialized.");
	}
	return *data_;
}

void MaterialInstance::SetBlendMode(BlendMode blendMode) {
	if (blendMode >= BlendMode::Count) {
		throw std::invalid_argument("Invalid material blend mode.");
	}
	blendMode_ = blendMode;
}
