#pragma once

#include <cstdint>
#include <d3d12.h>
#include <wrl.h>
#include <vector>

#include "engine/core/Mymath.h"

class DirectXCommon;

/// <summary>頂点データとGPU頂点バッファを所有する。</summary>
class Mesh {
public:
	void Initialize(DirectXCommon* dxCommon, const std::vector<VertexData>& vertices);
	void Draw(ID3D12GraphicsCommandList* commandList) const;
	bool IsInitialized() const { return vertexResource_ != nullptr; }
	uint32_t GetVertexCount() const { return vertexCount_; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	uint32_t vertexCount_ = 0;
};
