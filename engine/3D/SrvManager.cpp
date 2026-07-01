#include "SrvManager.h"

#include <cassert>

#include "engine/base/DirectXCommon.h"

void SrvManager::Initialize(DirectXCommon* dxCommon) {
	assert(dxCommon != nullptr);
	dxCommon_ = dxCommon;
	descriptorHeap_ = dxCommon_->CreateDescriptorHeap(
		dxCommon_->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
	descriptorSize_ = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t SrvManager::Allocate() {
	assert(useIndex_ < kMaxSRVCount);
	uint32_t index = useIndex_;
	++useIndex_;
	return index;
}

void SrvManager::PreDraw() {
	assert(dxCommon_ != nullptr);
	assert(descriptorHeap_ != nullptr);
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap_.Get() };
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* resource, const DirectX::TexMetadata& metadata) {
	assert(dxCommon_ != nullptr);
	assert(resource != nullptr);
	assert(srvIndex < kMaxSRVCount);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	dxCommon_->GetDevice()->CreateShaderResourceView(resource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index) const {
	assert(descriptorHeap_ != nullptr);
	return DirectXCommon::GetCPUDescriptorHandle(descriptorHeap_, descriptorSize_, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index) const {
	assert(descriptorHeap_ != nullptr);
	return DirectXCommon::GetGPUDescriptorHandle(descriptorHeap_, descriptorSize_, index);
}

ID3D12DescriptorHeap* SrvManager::GetDescriptorHeap() const {
	return descriptorHeap_.Get();
}
