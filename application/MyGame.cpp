#include "application/MyGame.h"

#include "engine/2d/Sprite.h"
#include "engine/3d/Camera.h"
#include "engine/3d/MeshGenerator.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/TextureManager.h"
#include "engine/core/DirectXCommon.h"
#include "engine/core/FrameRateController.h"
#include "engine/core/HResult.h"
#include "engine/core/ImGuiManager.h"
#include "engine/core/Input.h"
#include "engine/core/Time.h"
#include "engine/core/WinApp.h"

#include <stdexcept>
#include <cstring>

MyGame::MyGame() = default;

MyGame::~MyGame() { Finalize(); }

void MyGame::Initialize() {
	if (initialized_ || sprite_ || object3d_ || camera_ || vertexResourceSphere_ ||
		wvpResourceSphere_ || materialResourceSphere_ || directionalLightSphereResource_) {
		throw std::logic_error("MyGame is already initialized or partially initialized.");
	}

	try {
		// 最初にゲーム共通機能を初期化する
		Framework::Initialize();

	// このゲームで使用する音声を読み込む
	fanfareSound_ = audio_->Load("fanfare.wav");

	// このゲームで表示するスプライトの初期化
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(spriteCommon_.get());

	// 描画で切り替えて使用するテクスチャを読み込む
	const uint32_t uvCheckerTexture = textureManager_->Load("resource/uvChecker.png");
	const uint32_t monsterBallTexture = textureManager_->Load("resource/monsterBall.png");
	textureSrvHandleGPU_ = textureManager_->GetSrvHandleGPU(uvCheckerTexture);
	textureSrvHandleGPU2_ = textureManager_->GetSrvHandleGPU(monsterBallTexture);

	// 3Dオブジェクトとカメラの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon_.get(), textureManager_.get());
	camera_ = std::make_unique<Camera>();
	camera_->Update();

	// 球体の頂点バッファを作成し、CPUから書き込めるようにマップする
	const std::vector<VertexData> sphereVertices =
		MeshGenerator::CreateSphere(kSphereSubdivisions);
	sphereVertexCount_ = static_cast<uint32_t>(sphereVertices.size());
	vertexResourceSphere_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * sphereVertices.size());
	vertexBufferViewSphere_.BufferLocation = vertexResourceSphere_->GetGPUVirtualAddress();
	vertexBufferViewSphere_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * sphereVertices.size());
	vertexBufferViewSphere_.StrideInBytes = sizeof(VertexData);
	VertexData* vertexDataSphere = nullptr;
	HResult::ThrowIfFailed(
		vertexResourceSphere_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere)),
		"Mapping the sphere vertex buffer");
	std::memcpy(vertexDataSphere, sphereVertices.data(), sizeof(VertexData) * sphereVertices.size());

	// 球体のワールド・WVP行列用定数バッファ
	wvpResourceSphere_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
	HResult::ThrowIfFailed(
		wvpResourceSphere_->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSphere_)),
		"Mapping the sphere transformation buffer");
	wvpDataSphere_->World = MakeIdentity4x4();

	// 球体のマテリアル用定数バッファ
	materialResourceSphere_ = dxCommon_->CreateBufferResource(sizeof(Material));
	HResult::ThrowIfFailed(
		materialResourceSphere_->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere_)),
		"Mapping the sphere material buffer");
	materialDataSphere_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	materialDataSphere_->enableLighting = true;
	materialDataSphere_->uvTransform = MakeIdentity4x4();

	// 球体に当てる平行光源用定数バッファ
	directionalLightSphereResource_ = dxCommon_->CreateBufferResource(sizeof(DirectionalLight));
	HResult::ThrowIfFailed(
		directionalLightSphereResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightSphereData_)),
		"Mapping the sphere directional-light buffer");
	directionalLightSphereData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLightSphereData_->direction = {0.0f, -1.0f, 0.0f};
	directionalLightSphereData_->intensity = 1.0f;

	// 毎フレーム更新するスプライトの定数バッファを取得
	materialDataSprite_ = sprite_->GetMaterialData();
	transformationMatrixDataSprite_ = sprite_->GetTransformationMatrixData();
		initialized_ = true;
	}
	catch (...) {
		Finalize();
		throw;
	}
}

void MyGame::Update() {
#ifdef _DEBUG
	// ImGuiで描画対象や座標、ライト、音声を操作する
	imguiManager_->BeginFrame();
	imguiManager_->DrawDebugWindow(isModel_, isSphere_, isRotate_, isSprite_, textureChange_,
		*materialDataSphere_, transformSphere_, *directionalLightSphereData_, transformSprite_,
		uvTransformSprite_, *audio_, fanfareSound_, *frameRateController_);
#endif

	// 入力と音声など、ゲーム共通の毎フレーム処理
	Framework::Update();

	// ここからこのゲーム固有の更新処理
	if (input_->TriggerKey(DIK_0)) { audio_->Play(fanfareSound_, false, 1.0f, 1.0f); }
	constexpr float kSphereRotationSpeed = 3.0f; // radians per second
	if (isRotate_) {
		transformSphere_.rotate.y -= kSphereRotationSpeed * time_->GetDeltaTime();
	}

	// 3Dモデルのワールド・ビュー・プロジェクション行列を更新
	const Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	camera_->Update();
	const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
	object3d_->GetTransformationMatrixData()->World = worldMatrix;
	object3d_->GetTransformationMatrixData()->WVP = Multiply(worldMatrix, viewProjectionMatrix);
	object3d_->GetDirectionalLightData()->direction = Normalize(object3d_->GetDirectionalLightData()->direction);

	// 球体の行列を更新（頂点データは初期化時に一度だけ生成済み）
	const Matrix4x4 worldMatrixSphere = MakeAffineMatrix(transformSphere_.scale, transformSphere_.rotate, transformSphere_.translate);
	wvpDataSphere_->World = worldMatrixSphere;
	wvpDataSphere_->WVP = Multiply(worldMatrixSphere, viewProjectionMatrix);
	directionalLightSphereData_->direction = Normalize(directionalLightSphereData_->direction);

	// 画面座標系でスプライトの行列を更新
	const Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite_.scale, transformSprite_.rotate, transformSprite_.translate);
	const Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f,
		static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight), 0.0f, 100.0f);
	transformationMatrixDataSprite_->WVP = Multiply(worldMatrixSprite, projectionMatrixSprite);

	// スプライトに適用するUV変換を更新
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite_.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite_.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite_.translate));
	materialDataSprite_->uvTransform = uvTransformMatrix;
#ifdef _DEBUG
	imguiManager_->EndFrame();
#endif
}

void MyGame::Draw() {
	// Framework側でフレームの描画準備を行う
	BeginDraw();
	object3dCommon_->CommonDrawSetting();

	// 球体を描画
	if (isSphere_) {
		dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSphere_);
		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSphere_->GetGPUVirtualAddress());
		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResourceSphere_->GetGPUVirtualAddress());
		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureChange_ ? textureSrvHandleGPU2_ : textureSrvHandleGPU_);
		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightSphereResource_->GetGPUVirtualAddress());
		dxCommon_->GetCommandList()->DrawInstanced(sphereVertexCount_, 1, 0, 0);
	}

	// OBJモデルを描画
	if (isModel_) { object3d_->Draw(); }

	// 2Dスプライトを描画
	if (isSprite_) {
		spriteCommon_->CommonDrawSetting();
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite = sprite_->GetVertexBufferView();
		D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite = sprite_->GetIndexBufferView();
		dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		dxCommon_->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);
		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, sprite_->GetMaterialResource()->GetGPUVirtualAddress());
		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, sprite_->GetTransformationMatrixResource()->GetGPUVirtualAddress());
		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureChange_ ? textureSrvHandleGPU2_ : textureSrvHandleGPU_);
		dxCommon_->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}
	// ImGuiの描画と画面表示はFramework側で行う
	EndDraw();
}

void MyGame::Finalize() {
	initialized_ = false;

	// このゲーム固有のオブジェクトを先に解放する
	sprite_.reset();
	object3d_.reset();
	camera_.reset();

	// DirectXCommonを破棄する前にGPUリソースを解放する
	vertexResourceSphere_.Reset();
	wvpResourceSphere_.Reset();
	materialResourceSphere_.Reset();
	directionalLightSphereResource_.Reset();
	vertexBufferViewSphere_ = {};
	sphereVertexCount_ = 0;
	wvpDataSphere_ = nullptr;
	materialDataSphere_ = nullptr;
	directionalLightSphereData_ = nullptr;
	materialDataSprite_ = nullptr;
	transformationMatrixDataSprite_ = nullptr;
	fanfareSound_ = Audio::kInvalidSoundHandle;
	textureSrvHandleGPU_ = {};
	textureSrvHandleGPU2_ = {};

	// 最後にゲーム共通機能を解放する
	Framework::Finalize();
}
