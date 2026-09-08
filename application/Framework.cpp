#include "application/Framework.h"

#include "engine/2d/SpriteCommon.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/SrvManager.h"
#include "engine/3d/TextureManager.h"
#include "engine/audio/Audio.h"
#include "engine/core/DirectXCommon.h"
#include "engine/core/ImGuiManager.h"
#include "engine/core/Input.h"
#include "engine/core/WinApp.h"

Framework::~Framework() {
	Finalize();
}

void Framework::Initialize() {
	// Windowsアプリケーションと入力の初期化
	winApp_ = new WinApp();
	winApp_->Initialize();
	input_ = new Input();
	input_->Initialize(winApp_);

	// ゲーム内で共有する音声システムの初期化
	audio_ = new Audio();
	audio_->Initialize("resource/audio");

	// DirectXとGPUディスクリプタ管理の初期化
	dxCommon_ = new DirectXCommon();
	dxCommon_->Initialize(winApp_);
	srvManager_ = new SrvManager();
	srvManager_->Initialize(dxCommon_);
	textureManager_ = new TextureManager();
	textureManager_->Initialize(dxCommon_, srvManager_);

	// 2D・3D描画で共通使用するパイプラインの初期化
	spriteCommon_ = new SpriteCommon();
	spriteCommon_->Initialize(dxCommon_);
	object3dCommon_ = new Object3dCommon();
	object3dCommon_->Initialize(dxCommon_);

#ifdef _DEBUG
	// デバッグビルド時のみImGuiを使用する
	imguiManager_ = new ImGuiManager();
	imguiManager_->Initialize(winApp_, dxCommon_);
#endif

	initialized_ = true;
}

void Framework::Update() {
	// すべてのゲームで必要になる毎フレーム処理
	input_->Update();
	audio_->Update();
}

bool Framework::IsEndRequest() {
	return winApp_->ProcessMessege();
}

void Framework::BeginDraw() {
	// バックバッファを描画可能な状態にしてSRVヒープを設定する
	dxCommon_->PreDraw();
	srvManager_->PreDraw();
}

void Framework::EndDraw() {
#ifdef _DEBUG
	// ゲーム画面の手前にImGuiを描画する
	imguiManager_->Draw(dxCommon_->GetCommandList());
#endif

	// 描画命令を実行して画面を表示する
	dxCommon_->PostDraw();
}

void Framework::Finalize() {
	if (!initialized_) {
		return;
	}

#ifdef _DEBUG
	imguiManager_->Finalize();
	delete imguiManager_;
	imguiManager_ = nullptr;
#endif

	// 依存される側が後まで残る順序で共通機能を解放する
	delete object3dCommon_;
	object3dCommon_ = nullptr;
	delete spriteCommon_;
	spriteCommon_ = nullptr;
	delete textureManager_;
	textureManager_ = nullptr;
	delete srvManager_;
	srvManager_ = nullptr;
	delete audio_;
	audio_ = nullptr;
	delete input_;
	input_ = nullptr;
	delete dxCommon_;
	dxCommon_ = nullptr;
	delete winApp_;
	winApp_ = nullptr;

	initialized_ = false;
}
