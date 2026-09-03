#pragma once

#include <cstdint>

#include <d3d12.h>
#include <wrl.h>

#include "externals/DirectXTex/DirectXTex.h"

class DirectXCommon;

class SrvManager {
public:
	static const uint32_t kMaxSRVCount = 128;

	void Initialize(DirectXCommon* dxCommon);
	uint32_t Allocate();
	void PreDraw();
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* resource, const DirectX::TexMetadata& metadata);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index) const;
	ID3D12DescriptorHeap* GetDescriptorHeap() const;

private:
	DirectXCommon* dxCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
	uint32_t descriptorSize_ = 0;
	uint32_t useIndex_ = 1;
};
