#include "Sprite.h"

#include <cassert>

void Sprite::Initialize(SpriteCommon* spriteCommon) {
	assert(spriteCommon != nullptr);
	assert(spriteCommon->GetDXCommon() != nullptr);

	spriteCommon_ = spriteCommon;
	DirectXCommon* dxCommon = spriteCommon_->GetDXCommon();

	vertexResource_ = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0].position = { 0.0f,360.0f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[3].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexData[3].texcoord = { 1.0f,0.0f };

	indexResource_ = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;

	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformationMatrixData_->World = MakeIdentity4x4();
	transformationMatrixData_->WVP = MakeIdentity4x4();

	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->enableLighting = false;
	materialData_->uvTransform = MakeIdentity4x4();
}

D3D12_VERTEX_BUFFER_VIEW Sprite::GetVertexBufferView() const { return vertexBufferView_; }

D3D12_INDEX_BUFFER_VIEW Sprite::GetIndexBufferView() const { return indexBufferView_; }

ID3D12Resource* Sprite::GetMaterialResource() const { return materialResource_.Get(); }

ID3D12Resource* Sprite::GetTransformationMatrixResource() const { return transformationMatrixResource_.Get(); }

Material* Sprite::GetMaterialData() const { return materialData_; }

TransformationMatrix* Sprite::GetTransformationMatrixData() const { return transformationMatrixData_; }