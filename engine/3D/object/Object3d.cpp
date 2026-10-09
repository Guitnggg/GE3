#include "Object3d.h"
#include "engine/math/MathUtility.h"

#include <stdexcept>

#include "engine/3D/camera/Camera.h"
#include "engine/3D/model/Model.h"
#include "Object3dCommon.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/graphics/material/MaterialInstance.h"
#include "engine/core/diagnostics/HResult.h"

void Object3d::Initialize(Object3dCommon *object3dCommon,
                          TextureManager *textureManager,
                          const std::shared_ptr<Model> &model) {
	// 描画に必要な管理クラスとモデルが揃っていることを確認する
	if (object3dCommon == nullptr || object3dCommon->GetDxCommon() == nullptr || textureManager == nullptr ||
	    model == nullptr) {
		throw std::invalid_argument("Object3d requires Object3dCommon, TextureManager, and Model.");
	}
	// モデルはshared_ptrで保持し、同じGPUメッシュを複数配置から共有する
	object3dCommon_ = object3dCommon;
	textureManager_ = textureManager;
	model_ = model;

	auto *dxCommon = object3dCommon_->GetDxCommon();
	material_ = std::make_shared<MaterialInstance>();
	material_->Initialize(dxCommon, model_->GetDefaultTextureIndex());

	// ワールド行列とWVP行列を毎フレーム更新する定数バッファを生成する
	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	HResult::ThrowIfFailed(
	    transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void **>(&transformationMatrixData_)),
	    "Mapping the 3D object transformation buffer");
	transformationMatrixData_->World = MakeIdentity4x4();
	transformationMatrixData_->WVP = MakeIdentity4x4();

	// この配置に適用する平行光源用定数バッファを生成する
	directionalLightResource_ = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	HResult::ThrowIfFailed(
	    directionalLightResource_->Map(0, nullptr, reinterpret_cast<void **>(&directionalLightData_)),
	    "Mapping the 3D object directional-light buffer");
	directionalLightData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLightData_->direction = {0.0f, -1.0f, 0.0f};
	directionalLightData_->intensity = 1.0f;
}

void Object3d::Update(const Camera &camera) {
	// 初期化前は行列の書き込み先を持たないため更新を拒否する
	if (transformationMatrixData_ == nullptr || directionalLightData_ == nullptr) {
		throw std::logic_error("Object3d is not initialized.");
	}

	// 配置情報からワールド行列を作り、カメラのViewProjectionと合成する
	const Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->WVP = Multiply(worldMatrix, camera.GetViewProjectionMatrix());

	// シェーダーへ常に単位ベクトルを渡せるようライト方向を正規化する
	directionalLightData_->direction = Normalize(directionalLightData_->direction);
}

void Object3d::Draw() const {
	// 未初期化状態でGPUコマンドを記録しないよう検証する
	if (object3dCommon_ == nullptr || textureManager_ == nullptr || model_ == nullptr || material_ == nullptr) {
		throw std::logic_error("Object3d is not initialized.");
	}
	// ルートパラメータ0～3へ、マテリアル・行列・テクスチャ・ライトを順番に設定する
	auto *commandList = object3dCommon_->GetDxCommon()->GetCommandList();
	object3dCommon_->SetBlendMode(material_->GetBlendMode());
	material_->Bind(commandList, *textureManager_);
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	// 共有モデルが所有するメッシュの頂点バッファを設定して描画する
	model_->Draw(commandList);
}

void Object3d::SetMaterial(const std::shared_ptr<MaterialInstance> &material) {
	if (material == nullptr || !material->IsInitialized()) {
		throw std::invalid_argument("Object3d requires an initialized material.");
	}
	material_ = material;
}
