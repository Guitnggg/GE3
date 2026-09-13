#include "Mesh.h"

#include <cstring>
#include <limits>
#include <stdexcept>

#include "engine/core/DirectXCommon.h"
#include "engine/core/HResult.h"

void Mesh::Initialize(DirectXCommon* dxCommon, const std::vector<VertexData>& vertices) {
	if (dxCommon == nullptr) { throw std::invalid_argument("Mesh requires DirectXCommon."); }
	if (vertices.empty()) { throw std::invalid_argument("Mesh requires at least one vertex."); }
	if (vertices.size() > (std::numeric_limits<uint32_t>::max)() ||
		vertices.size() > (std::numeric_limits<UINT>::max)() / sizeof(VertexData)) {
		throw std::overflow_error("Mesh vertex data is too large.");
	}

	vertexResource_ = dxCommon->CreateBufferResource(sizeof(VertexData) * vertices.size());
	VertexData* mappedVertices = nullptr;
	HResult::ThrowIfFailed(
		vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertices)),
		"Mapping the mesh vertex buffer");
	std::memcpy(mappedVertices, vertices.data(), sizeof(VertexData) * vertices.size());

	vertexCount_ = static_cast<uint32_t>(vertices.size());
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void Mesh::Draw(ID3D12GraphicsCommandList* commandList) const {
	if (commandList == nullptr || !IsInitialized()) {
		throw std::logic_error("Mesh is not initialized or has no command list.");
	}
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->DrawInstanced(vertexCount_, 1, 0, 0);
}
