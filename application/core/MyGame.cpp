#include "application/core/MyGame.h"

#include "application/scenes/GameScene.h"
#include "engine/scene/SceneManager.h"

#include <stdexcept>

MyGame::MyGame() = default;
MyGame::~MyGame() { Finalize(); }

void MyGame::Initialize() {
	// 多重初期化や前回の不完全な初期化状態を検出する
	if (initialized_ || sceneManager_) {
		throw std::logic_error("MyGame is already initialized or partially initialized.");
	}

	try {
		// シーンより先に、シーンから参照されるエンジン共通機能を初期化する
		Framework::Initialize();

		// Frameworkが所有する各機能を、所有権を移さずシーンへ公開する
		SceneContext context{};
		context.audio = audio_.get();
		context.input = input_.get();
		context.textureManager = textureManager_.get();
		context.modelManager = modelManager_.get();
		context.spriteCommon = spriteCommon_.get();
		context.object3dCommon = object3dCommon_.get();
		context.time = time_.get();
		context.frameRateController = frameRateController_.get();
#ifdef _DEBUG
		context.imguiManager = imguiManager_.get();
#endif

		// シーン管理を生成し、最初のゲームシーンを開始する
		sceneManager_ = std::make_unique<SceneManager>();
		sceneManager_->Initialize(context, std::make_unique<GameScene>());
		initialized_ = true;
	} catch (...) {
		// 途中まで生成したリソースも通常の終了順序で片付ける
		Finalize();
		throw;
	}
}

void MyGame::Update() {
	// 入力・時間などの共通機能を更新してから、ゲーム固有の処理を進める
	Framework::Update();
	sceneManager_->Update();
}

void MyGame::FixedUpdate() {
	// 初期化失敗後の終了処理でも安全に呼べるよう存在を確認する
	if (sceneManager_) { sceneManager_->FixedUpdate(); }
}

void MyGame::Draw() {
	// バックバッファの準備と表示処理の間に、現在のシーンを描画する
	BeginDraw();
	sceneManager_->Draw();
	EndDraw();
}

void MyGame::Finalize() {
	initialized_ = false;

	// シーンが参照する共通機能より先にシーンを終了する
	if (sceneManager_) { sceneManager_->Finalize(); }
	sceneManager_.reset();
	Framework::Finalize();
}
