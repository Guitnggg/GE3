#include "Sprite.h"

#include <cmath>
#include <stdexcept>

#include "SpriteCommon.h"
#include "engine/3D/TextureManager.h"
#include "engine/core/HResult.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, TextureManager* textureManager, uint32_t textureIndex) {
	// 描画パイプラインとテクスチャ管理の両方が利用可能か確認する
	if (spriteCommon == nullptr || spriteCommon->GetDXCommon() == nullptr || textureManager == nullptr) {
		throw std::invalid_argument("Sprite requires SpriteCommon and TextureManager.");
	}
	spriteCommon_ = spriteCommon;
	textureManager_ = textureManager;
	textureIndex_ = textureIndex;
	DirectXCommon* dxCommon = spriteCommon_->GetDXCommon();

	// CPUから毎フレーム書き換えられる矩形頂点バッファを生成する
	vertexResource_ = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	HResult::ThrowIfFailed(
		vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_)),
		"Mapping the sprite vertex buffer");
	UpdateVertexData();

	// 矩形を2枚の三角形として描画するインデックスバッファを生成する
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

	// 画面座標変換用の定数バッファを生成し、単位行列で初期化する
	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	HResult::ThrowIfFailed(
		transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_)),
		"Mapping the sprite transformation buffer");
	transformationMatrixData_->World = MakeIdentity4x4();
	transformationMatrixData_->WVP = MakeIdentity4x4();

	// スプライト色とUV変換をシェーダーへ渡す定数バッファを生成する
	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	HResult::ThrowIfFailed(
		materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_)),
		"Mapping the sprite material buffer");
	materialData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	materialData_->enableLighting = false;
	materialData_->uvTransform = MakeIdentity4x4();
}

void Sprite::Update(float viewportWidth, float viewportHeight) {
	// 正しい正射影行列を作れない画面サイズや未初期化状態を拒否する
	if (transformationMatrixData_ == nullptr || materialData_ == nullptr) {
		throw std::logic_error("Sprite is not initialized.");
	}
	if (!std::isfinite(viewportWidth) || !std::isfinite(viewportHeight) ||
		viewportWidth <= 0.0f || viewportHeight <= 0.0f) {
		throw std::invalid_argument("Sprite viewport size must be finite and positive.");
	}
	// SetSizeやSetAnchorPointで変更された矩形形状を頂点バッファへ反映する
	UpdateVertexData();

	// 左上を原点とする画面座標へ、スプライト自身のTransformを適用する
	const Matrix4x4 worldMatrix =
		MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	const Matrix4x4 projectionMatrix =
		MakeOrthographicMatrix(0.0f, 0.0f, viewportWidth, viewportHeight, 0.0f, 100.0f);
	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->WVP = Multiply(worldMatrix, projectionMatrix);

	// 拡縮、Z回転、移動の順にUV変換行列を合成する
	Matrix4x4 uvMatrix = MakeScaleMatrix(uvTransform_.scale);
	uvMatrix = Multiply(uvMatrix, MakeRotateZMatrix(uvTransform_.rotate.z));
	uvMatrix = Multiply(uvMatrix, MakeTranslateMatrix(uvTransform_.translate));
	materialData_->uvTransform = uvMatrix;
}

void Sprite::Draw() const {
	// 描画に必要な管理クラスとGPUリソースが揃っているか確認する
	if (spriteCommon_ == nullptr || textureManager_ == nullptr ||
		materialResource_ == nullptr || transformationMatrixResource_ == nullptr) {
		throw std::logic_error("Sprite is not initialized.");
	}

	// スプライト用パイプラインを設定し、各バッファとテクスチャをバインドする
	spriteCommon_->CommonDrawSetting();
	auto* commandList = spriteCommon_->GetDXCommon()->GetCommandList();
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(textureIndex_));
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::SetSize(const Vector2& size) {
	if (!std::isfinite(size.x) || !std::isfinite(size.y) || size.x <= 0.0f || size.y <= 0.0f) {
		throw std::invalid_argument("Sprite size must be finite and positive.");
	}
	size_ = size;
}

void Sprite::UpdateVertexData() {
	if (vertexData_ == nullptr) { throw std::logic_error("Sprite vertex buffer is not mapped."); }
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
