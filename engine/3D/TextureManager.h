#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <d3d12.h>
#include <wrl.h>

#include "externals/DirectXTex/DirectXTex.h"

class DirectXCommon;
class SrvManager;

class TextureManager {
public:
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	uint32_t Load(const std::string& filePath);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex) const;

private:
	struct TextureData {
		DirectX::TexMetadata metadata{};
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex = 0;
	};

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	std::vector<TextureData> textures_;
	std::unordered_map<std::string, uint32_t> textureIndexMap_;
};
