#include "Sprite.h"

#include <cmath>
#include <cstring>
#include <stdexcept>

#include "SpriteCommon.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/core/diagnostics/HResult.h"

void Sprite::Initialize(SpriteCommon *spriteCommon, TextureManager *textureManager, TextureHandle textureHandle) {
	// 描画パイプラインとテクスチャ管理の両方が利用可能か確認する
	if (spriteCommon == nullptr || spriteCommon->GetDXCommon() == nullptr || textureManager == nullptr) {
		throw std::invalid_argument("Sprite requires SpriteCommon and TextureManager.");
	}
	spriteCommon_ = spriteCommon;
	textureManager_ = textureManager;
	textureHandle_ = textureHandle;
	DirectXCommon *dxCommon = spriteCommon_->GetDXCommon();

	// 毎フレーム変化する頂点はCPU側に保持し、Draw時にFrameContext専用領域へ転送する。
	UpdateVertexData();

	// 矩形を2枚の三角形として描画するインデックスバッファを生成する
	indexResource_ = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	uint32_t *indexData = nullptr;
	HResult::ThrowIfFailed(indexResource_->Map(0, nullptr, reinterpret_cast<void **>(&indexData)),
	                       "Mapping the sprite index buffer");
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;

	transformationMatrixData_.World = MakeIdentity4x4();
	transformationMatrixData_.WVP = MakeIdentity4x4();
	materialData_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	materialData_.enableLighting = false;
	materialData_.uvTransform = MakeIdentity4x4();
}

void Sprite::Update(float viewportWidth, float viewportHeight) {
	// 正しい正射影行列を作れない画面サイズや未初期化状態を拒否する
	if (spriteCommon_ == nullptr || textureManager_ == nullptr) {
		throw std::logic_error("Sprite is not initialized.");
	}
	if (!std::isfinite(viewportWidth) || !std::isfinite(viewportHeight) || viewportWidth <= 0.0f ||
	    viewportHeight <= 0.0f) {
		throw std::invalid_argument("Sprite viewport size must be finite and positive.");
	}
	// SetSizeやSetAnchorPointで変更された矩形形状を頂点バッファへ反映する
	UpdateVertexData();

	// 左上を原点とする画面座標へ、スプライト自身のTransformを適用する
	const Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	const Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, viewportWidth, viewportHeight, 0.0f, 100.0f);
	transformationMatrixData_.World = worldMatrix;
	transformationMatrixData_.WVP = Multiply(worldMatrix, projectionMatrix);

	// 拡縮、Z回転、移動の順にUV変換行列を合成する
	Matrix4x4 uvMatrix = MakeScaleMatrix(uvTransform_.scale);
	uvMatrix = Multiply(uvMatrix, MakeRotateZMatrix(uvTransform_.rotate.z));
	uvMatrix = Multiply(uvMatrix, MakeTranslateMatrix(uvTransform_.translate));
	materialData_.uvTransform = uvMatrix;
}

void Sprite::Draw() const {
	// 描画に必要な管理クラスとGPUリソースが揃っているか確認する
	if (spriteCommon_ == nullptr || textureManager_ == nullptr || indexResource_ == nullptr) {
		throw std::logic_error("Sprite is not initialized.");
	}

	// スプライト用パイプラインを設定し、各バッファとテクスチャをバインドする
	spriteCommon_->CommonDrawSetting();
	DirectXCommon *dxCommon = spriteCommon_->GetDXCommon();
	auto *commandList = dxCommon->GetCommandList();
	const auto vertices = dxCommon->AllocateDynamicBuffer(sizeof(vertexData_), alignof(VertexData));
	const auto material = dxCommon->AllocateDynamicBuffer(sizeof(materialData_));
	const auto transform = dxCommon->AllocateDynamicBuffer(sizeof(transformationMatrixData_));
	std::memcpy(vertices.cpuAddress, vertexData_.data(), sizeof(vertexData_));
	std::memcpy(material.cpuAddress, &materialData_, sizeof(materialData_));
	std::memcpy(transform.cpuAddress, &transformationMatrixData_, sizeof(transformationMatrixData_));
	const D3D12_VERTEX_BUFFER_VIEW vertexBufferView{
	    vertices.gpuAddress, static_cast<UINT>(vertices.size), sizeof(VertexData)};
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->SetGraphicsRootConstantBufferView(0, material.gpuAddress);
	commandList->SetGraphicsRootConstantBufferView(1, transform.gpuAddress);
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(textureHandle_));
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::SetSize(const Vector2 &size) {
	if (!std::isfinite(size.x) || !std::isfinite(size.y) || size.x <= 0.0f || size.y <= 0.0f) {
		throw std::invalid_argument("Sprite size must be finite and positive.");
	}
	size_ = size;
}

void Sprite::UpdateVertexData() {
	const float left = -anchorPoint_.x * size_.x;
	const float right = left + size_.x;
	const float top = -anchorPoint_.y * size_.y;
	const float bottom = top + size_.y;

	// 左下、左上、右下、右上の順で位置とUVを設定する
	vertexData_[0] = {{left, bottom, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}};
	vertexData_[1] = {{left, top, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}};
	vertexData_[2] = {{right, bottom, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}};
	vertexData_[3] = {{right, top, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}};
}
