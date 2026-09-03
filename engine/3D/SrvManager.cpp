#include "SrvManager.h"

#include <cassert>
#include <stdexcept>

#include "engine/core/DirectXCommon.h"

void SrvManager::Initialize(DirectXCommon* dxCommon) {
	if (dxCommon == nullptr || dxCommon->GetSRVDescriptorHeap() == nullptr) {
		throw std::invalid_argument("SrvManager requires an initialized DirectXCommon instance.");
	}
	dxCommon_ = dxCommon;
}

uint32_t SrvManager::Allocate() {
	if (useIndex_ >= kMaxSRVCount) {
		throw std::runtime_error("The SRV descriptor heap is full.");
	}
	uint32_t index = useIndex_;
	++useIndex_;
	return index;
}

void SrvManager::PreDraw() {
	if (dxCommon_ == nullptr || dxCommon_->GetSRVDescriptorHeap() == nullptr) {
		throw std::logic_error("SrvManager is not initialized.");
	}
	ID3D12DescriptorHeap* descriptorHeaps[] = { dxCommon_->GetSRVDescriptorHeap() };
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* resource, const DirectX::TexMetadata& metadata) {
	if (dxCommon_ == nullptr || resource == nullptr || srvIndex >= kMaxSRVCount) {
		throw std::invalid_argument("Invalid texture SRV creation request.");
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	dxCommon_->GetDevice()->CreateShaderResourceView(resource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index) const {
	if (dxCommon_ == nullptr || index >= kMaxSRVCount) {
		throw std::out_of_range("Invalid SRV descriptor index.");
	}
	return dxCommon_->GetCPUDescriptorHandleSRV(index);
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index) const {
	if (dxCommon_ == nullptr || index >= kMaxSRVCount) {
		throw std::out_of_range("Invalid SRV descriptor index.");
	}
	return dxCommon_->GetGPUDescriptorHandleSRV(index);
}

ID3D12DescriptorHeap* SrvManager::GetDescriptorHeap() const {
	return dxCommon_ != nullptr ? dxCommon_->GetSRVDescriptorHeap() : nullptr;
}
