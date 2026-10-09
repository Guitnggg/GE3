#include "engine/core/Engine.h"

#include "engine/2d/SpriteCommon.h"
#include "engine/3D/object/Object3dCommon.h"
#include "engine/3D/model/ModelManager.h"
#include "engine/graphics/resource/SrvManager.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/audio/Audio.h"
#include "engine/assets/AssetManager.h"
#include "engine/collision/CollisionWorld.h"
#include "engine/core/DirectXCommon.h"
#include "engine/debug/DebugOverlay.h"
#include "engine/debug/Profiler.h"
#include "engine/effects/particle/GPUParticlePipeline.h"
#include "engine/graphics/shader/ShaderCompiler.h"
#include "engine/core/timing/FrameRateController.h"
#include "engine/graphics/debug/ImGuiManager.h"
#include "engine/input/Input.h"
#include "engine/settings/EngineSettings.h"
#include "engine/core/timing/Time.h"
#include "engine/core/WinApp.h"

#include <stdexcept>

Engine::Engine() = default;

Engine::~Engine() {
	Finalize();
}

void Engine::Run() {
	Initialize();
	bool applicationInitialized = false;

	try {
		// 失敗途中でもOnFinalizeでゲーム側のリソースを片付けられるよう、呼び出し前に状態を立てる
		applicationInitialized = true;
		OnInitialize();

		while (!IsEndRequest()) {
			BeginFrame();
			OnUpdate();
			BeginDraw();
			OnDraw();
			EndDraw();
		}

		applicationInitialized = false;
		OnFinalize();
		Finalize();
	} catch (...) {
		if (applicationInitialized) {
			try {
				OnFinalize();
			} catch (...) {
				// 最初に発生した例外を維持したままエンジン共通リソースを解放する
			}
		}
		Finalize();
		throw;
	}
}

void Engine::Initialize() {
	if (initialized_ || winApp_ || input_ || audio_ || collisionWorld_ || dxCommon_ || srvManager_ || textureManager_ ||
	    modelManager_ || spriteCommon_ || object3dCommon_ || time_ || frameRateController_ || gpuParticlePipeline_ ||
	    shaderCompiler_ || assetManager_ || engineSettings_
#ifdef _DEBUG
	    || imguiManager_ || debugOverlay_
#endif
	) {
		throw std::logic_error("Engine is already initialized or partially initialized.");
	}

	try {
		// ゲーム時間の計測を初期化する
		time_ = std::make_unique<Time>();
		time_->Initialize();
		frameRateController_ = std::make_unique<FrameRateController>();
		frameRateController_->Initialize();
		engineSettings_ = std::make_unique<EngineSettings>();
		std::string settingsError;
		if (engineSettings_->Load("resource/config/engine.json", &settingsError)) {
			engineSettings_->Apply(*frameRateController_, *time_);
		}

		// Windowsアプリケーションと入力の初期化
		winApp_ = std::make_unique<WinApp>();
		winApp_->Initialize();
		input_ = std::make_unique<Input>();
		input_->Initialize(winApp_.get());

		// ゲーム内で共有する音声システムの初期化
		audio_ = std::make_unique<Audio>();
		audio_->Initialize("resource/audio");
		collisionWorld_ = std::make_unique<CollisionWorld>();

		// DirectXとGPUディスクリプタ管理の初期化
		dxCommon_ = std::make_unique<DirectXCommon>();
		dxCommon_->Initialize(winApp_.get());
		shaderCompiler_ = std::make_unique<ShaderCompiler>();
		shaderCompiler_->Initialize();
		srvManager_ = std::make_unique<SrvManager>();
		srvManager_->Initialize(dxCommon_.get());
		textureManager_ = std::make_unique<TextureManager>();
		textureManager_->Initialize(dxCommon_.get(), srvManager_.get());
		modelManager_ = std::make_unique<ModelManager>();
		modelManager_->Initialize(dxCommon_.get(), textureManager_.get());
		assetManager_ = std::make_unique<AssetManager>();
		assetManager_->Initialize(textureManager_.get(), modelManager_.get(), audio_.get());

		// 2D・3D描画で共通使用するパイプラインの初期化
		spriteCommon_ = std::make_unique<SpriteCommon>();
		spriteCommon_->Initialize(dxCommon_.get(), shaderCompiler_.get());
		object3dCommon_ = std::make_unique<Object3dCommon>();
		object3dCommon_->Initialize(dxCommon_.get(), shaderCompiler_.get());
		gpuParticlePipeline_ = std::make_unique<GPUParticlePipeline>();
		gpuParticlePipeline_->Initialize(dxCommon_.get(), shaderCompiler_.get());

#ifdef _DEBUG
		// デバッグビルド時のみImGuiを使用する
		imguiManager_ = std::make_unique<ImGuiManager>();
		imguiManager_->Initialize(winApp_.get(), dxCommon_.get());
		debugOverlay_ = std::make_unique<DebugOverlay>();
		debugOverlay_->Initialize(frameRateController_.get(), time_.get());
#endif

		initialized_ = true;
	} catch (...) {
		Engine::Finalize();
		throw;
	}
}

void Engine::BeginFrame() {
#ifdef _DEBUG
	Profiler::Get().BeginFrame();
	PROFILE_SCOPE("Engine BeginFrame");
#endif
	// すべてのゲームで必要になる毎フレーム処理を先に更新する
	frameRateController_->BeginFrame();
	time_->Update();
	input_->Update();
	audio_->Update();

#ifdef _DEBUG
	// デバッグUIのフレーム管理はゲームシーンの有無に依存させない
	imguiManager_->BeginFrame();
#endif

	// 蓄積時間が固定間隔を満たす間、物理・固定ロジックを一定刻みで進める
	while (time_->ConsumeFixedStep()) {
		OnFixedUpdate();
	}
}

void Engine::OnInitialize() {}
void Engine::OnUpdate() {}
void Engine::OnFixedUpdate() {}
void Engine::OnDraw() {}
void Engine::OnFinalize() {}

bool Engine::IsEndRequest() {
	return winApp_->ProcessMessage();
}

void Engine::BeginDraw() {
	// バックバッファを描画可能な状態にし、描画に必要な状態を設定する
	dxCommon_->PreDraw();
}

void Engine::EndDraw() {
#ifdef _DEBUG
	debugOverlay_->Draw();
	Profiler::Get().EndFrame();
	// UI構築を確定してから、ゲーム画面の手前にImGuiを描画する
	imguiManager_->EndFrame();
	imguiManager_->Draw(dxCommon_->GetCommandList());
#endif

	// 描画命令を実行して画面を表示する
	dxCommon_->SetVSyncEnabled(frameRateController_->IsVSyncEnabled());
	dxCommon_->PostDraw();
	frameRateController_->EndFrame();
}

void Engine::Finalize() {
	initialized_ = false;

#ifdef _DEBUG
	if (debugOverlay_) {
		debugOverlay_->Finalize();
	}
	debugOverlay_.reset();
	if (imguiManager_) {
		imguiManager_->Finalize();
	}
	imguiManager_.reset();
#endif

	// 依存される側が後まで残る順序で共通機能を解放する
	assetManager_.reset();
	engineSettings_.reset();
	gpuParticlePipeline_.reset();
	object3dCommon_.reset();
	spriteCommon_.reset();
	shaderCompiler_.reset();
	modelManager_.reset();
	textureManager_.reset();
	srvManager_.reset();
	audio_.reset();
	collisionWorld_.reset();
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
