#include "application/core/Framework.h"

#include "engine/2d/SpriteCommon.h"
#include "engine/3D/object/Object3dCommon.h"
#include "engine/3D/model/ModelManager.h"
#include "engine/graphics/resource/SrvManager.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/audio/Audio.h"
#include "engine/core/DirectXCommon.h"
#include "engine/core/timing/FrameRateController.h"
#include "engine/graphics/debug/ImGuiManager.h"
#include "engine/input/Input.h"
#include "engine/core/timing/Time.h"
#include "engine/core/WinApp.h"

#include <stdexcept>

Framework::Framework() = default;

Framework::~Framework() {
	Finalize();
}

void Framework::Initialize() {
	if (initialized_ || winApp_ || input_ || audio_ || dxCommon_ || srvManager_ ||
		textureManager_ || modelManager_ || spriteCommon_ || object3dCommon_ || time_ || frameRateController_
#ifdef _DEBUG
		|| imguiManager_
#endif
	) {
		throw std::logic_error("Framework is already initialized or partially initialized.");
	}

	try {
	// ゲーム時間の計測を初期化する
	time_ = std::make_unique<Time>();
	time_->Initialize();
	frameRateController_ = std::make_unique<FrameRateController>();
	frameRateController_->Initialize();

	// Windowsアプリケーションと入力の初期化
	winApp_ = std::make_unique<WinApp>();
	winApp_->Initialize();
	input_ = std::make_unique<Input>();
	input_->Initialize(winApp_.get());

	// ゲーム内で共有する音声システムの初期化
	audio_ = std::make_unique<Audio>();
	audio_->Initialize("resource/audio");

	// DirectXとGPUディスクリプタ管理の初期化
	dxCommon_ = std::make_unique<DirectXCommon>();
	dxCommon_->Initialize(winApp_.get());
	srvManager_ = std::make_unique<SrvManager>();
	srvManager_->Initialize(dxCommon_.get());
	textureManager_ = std::make_unique<TextureManager>();
	textureManager_->Initialize(dxCommon_.get(), srvManager_.get());
	modelManager_ = std::make_unique<ModelManager>();
	modelManager_->Initialize(dxCommon_.get(), textureManager_.get());

	// 2D・3D描画で共通使用するパイプラインの初期化
	spriteCommon_ = std::make_unique<SpriteCommon>();
	spriteCommon_->Initialize(dxCommon_.get());
	object3dCommon_ = std::make_unique<Object3dCommon>();
	object3dCommon_->Initialize(dxCommon_.get());

#ifdef _DEBUG
	// デバッグビルド時のみImGuiを使用する
	imguiManager_ = std::make_unique<ImGuiManager>();
	imguiManager_->Initialize(winApp_.get(), dxCommon_.get());
#endif

	initialized_ = true;
	}
	catch (...) {
		Framework::Finalize();
		throw;
	}
}

void Framework::Update() {
	// すべてのゲームで必要になる毎フレーム処理を先に更新する
	frameRateController_->BeginFrame();
	time_->Update();
	input_->Update();
	audio_->Update();

	// 蓄積時間が固定間隔を満たす間、物理・固定ロジックを一定刻みで進める
	while (time_->ConsumeFixedStep()) {
		FixedUpdate();
	}
}

// 固定更新を使わないゲームもあるため、基底クラスの既定処理は空にする
void Framework::FixedUpdate() {}

bool Framework::IsEndRequest() {
	return winApp_->ProcessMessage();
}

void Framework::BeginDraw() {
	// バックバッファを描画可能な状態にし、描画に必要な状態を設定する
	dxCommon_->PreDraw();
}

void Framework::EndDraw() {
#ifdef _DEBUG
	// ゲーム画面の手前にImGuiを描画する
	imguiManager_->Draw(dxCommon_->GetCommandList());
#endif

	// 描画命令を実行して画面を表示する
	dxCommon_->SetVSyncEnabled(frameRateController_->IsVSyncEnabled());
	dxCommon_->PostDraw();
	frameRateController_->EndFrame();
}

void Framework::Finalize() {
	initialized_ = false;

#ifdef _DEBUG
	if (imguiManager_) {
		imguiManager_->Finalize();
	}
	imguiManager_.reset();
#endif

	// 依存される側が後まで残る順序で共通機能を解放する
	object3dCommon_.reset();
	spriteCommon_.reset();
	modelManager_.reset();
	textureManager_.reset();
	srvManager_.reset();
	audio_.reset();
	if (input_) {
		input_->Finalize();
	}
	input_.reset();
	dxCommon_.reset();
	if (winApp_) {
		winApp_->Finalize();
	}
	winApp_.reset();
	if (time_) {
		time_->Finalize();
	}
	time_.reset();
	if (frameRateController_) {
		frameRateController_->Finalize();
	}
	frameRateController_.reset();
}
