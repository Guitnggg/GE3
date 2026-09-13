#include "Sprite.h"

#include <stdexcept>

#include "engine/core/HResult.h"

// スプライト描画に必要な頂点、インデックス、マテリアル、行列リソースを初期化する
void Sprite::Initialize(SpriteCommon* spriteCommon) {
	if (spriteCommon == nullptr || spriteCommon->GetDXCommon() == nullptr) {
		throw std::invalid_argument("Sprite requires an initialized SpriteCommon.");
	}

	spriteCommon_ = spriteCommon;
	DirectXCommon* dxCommon = spriteCommon_->GetDXCommon();

	// 矩形スプライト用の頂点バッファを作成する
	vertexResource_ = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	HResult::ThrowIfFailed(
		vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData)),
		"Mapping the sprite vertex buffer");
	vertexData[0].position = { 0.0f,360.0f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[3].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexData[3].texcoord = { 1.0f,0.0f };

	// 2つの三角形で矩形を描くためのインデックスバッファを作成する
	indexResource_ = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexData = nullptr;
	HResult::ThrowIfFailed(
		indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData)),
		"Mapping the sprite index buffer");
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;

	// 座標変換行列用定数バッファを作成する
	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	HResult::ThrowIfFailed(
		transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_)),
		"Mapping the sprite transformation buffer");
	transformationMatrixData_->World = MakeIdentity4x4();
	transformationMatrixData_->WVP = MakeIdentity4x4();

	// マテリアル用定数バッファを作成する
	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	HResult::ThrowIfFailed(
		materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_)),
		"Mapping the sprite material buffer");
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
