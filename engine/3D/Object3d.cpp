#include "Object3d.h"

#include <stdexcept>

#include "Model.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "engine/core/HResult.h"

void Object3d::Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
	const std::shared_ptr<Model>& model) {
	// 描画に必要な管理クラスとモデルが揃っていることを確認する
	if (object3dCommon == nullptr || object3dCommon->GetDxCommon() == nullptr ||
		textureManager == nullptr || model == nullptr) {
		throw std::invalid_argument("Object3d requires Object3dCommon, TextureManager, and Model.");
	}
	// モデルはshared_ptrで保持し、同じGPUメッシュを複数配置から共有する
	object3dCommon_ = object3dCommon;
	textureManager_ = textureManager;
	model_ = model;
	textureIndex_ = model_->GetDefaultTextureIndex();

	auto* dxCommon = object3dCommon_->GetDxCommon();
	// この配置だけが持つマテリアル定数バッファを生成して初期値を書き込む
	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	HResult::ThrowIfFailed(materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_)),
		"Mapping the 3D object material buffer");
	materialData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	materialData_->enableLighting = true;
	materialData_->uvTransform = MakeIdentity4x4();

	// ワールド行列とWVP行列を毎フレーム更新する定数バッファを生成する
	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	HResult::ThrowIfFailed(
		transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_)),
		"Mapping the 3D object transformation buffer");
	transformationMatrixData_->World = MakeIdentity4x4();
	transformationMatrixData_->WVP = MakeIdentity4x4();

	// この配置に適用する平行光源用定数バッファを生成する
	directionalLightResource_ = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	HResult::ThrowIfFailed(
		directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_)),
		"Mapping the 3D object directional-light buffer");
	directionalLightData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLightData_->direction = {0.0f, -1.0f, 0.0f};
	directionalLightData_->intensity = 1.0f;
}

void Object3d::Draw() const {
	// 未初期化状態でGPUコマンドを記録しないよう検証する
	if (object3dCommon_ == nullptr || textureManager_ == nullptr || model_ == nullptr) {
		throw std::logic_error("Object3d is not initialized.");
	}
	// ルートパラメータ0～3へ、マテリアル・行列・テクスチャ・ライトを順番に設定する
	auto* commandList = object3dCommon_->GetDxCommon()->GetCommandList();
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(textureIndex_));
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	// 共有モデルが所有するメッシュの頂点バッファを設定して描画する
	model_->Draw(commandList);
}
