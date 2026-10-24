#include "engine/core/EngineCore.h"

#include "engine/2d/SpriteCommon.h"
#include "engine/3D/object/Object3dCommon.h"
#include "engine/3D/model/ModelManager.h"
#include "engine/graphics/resource/SrvManager.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/graphics/render/RenderTargetManager.h"
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
#include "engine/scene/IScene.h"
#include "engine/scene/AbstractSceneFactory.h"
#include "engine/scene/SceneContext.h"
#include "engine/scene/SceneManager.h"
#include "engine/core/timing/Time.h"
#include "engine/core/WinApp.h"

#include <stdexcept>

EngineCore::EngineCore() = default;

EngineCore::~EngineCore() {
	Finalize();
}

void EngineCore::Run() {
	Initialize();
	bool applicationInitialized = false;

	try {
		// 失敗途中でもOnFinalizeでゲーム側のリソースを片付けられるよう、呼び出し前に状態を立てる
		applicationInitialized = true;
		OnInitialize();

		while (!IsEndRequest()) {
			BeginFrame();

			// 通常更新で確定した状態だけを描画へ渡す。
			OnUpdate();
			sceneManager_->Update();

			BeginDraw();

			// Worldを先に描画し、その後に画面効果対象外のUIを重ねる。
			OnDrawWorld();
			sceneManager_->DrawWorld();

			BeginUIDraw();

			OnDrawUI();
			sceneManager_->DrawUI();

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

void EngineCore::Initialize() {
	if (initialized_ || winApp_ || input_ || audio_ || collisionWorld_ || dxCommon_ || srvManager_ ||
	    renderTargetManager_ || textureManager_ ||
	    modelManager_ || spriteCommon_ || object3dCommon_ || time_ || frameRateController_ || gpuParticlePipeline_ ||
	    shaderCompiler_ || assetManager_ || engineSettings_ || sceneManager_
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
		sceneManager_ = std::make_unique<SceneManager>();

		// DirectXとGPUディスクリプタ管理の初期化
		dxCommon_ = std::make_unique<DirectXCommon>();
		dxCommon_->Initialize(winApp_.get());
		shaderCompiler_ = std::make_unique<ShaderCompiler>();
		shaderCompiler_->Initialize();
		srvManager_ = std::make_unique<SrvManager>();
		srvManager_->Initialize(dxCommon_.get());
		renderTargetManager_ = std::make_unique<RenderTargetManager>();
		renderTargetManager_->Initialize(dxCommon_.get(), srvManager_.get(), shaderCompiler_.get());
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
		EngineCore::Finalize();
		throw;
	}
}

void EngineCore::BeginFrame() {
#ifdef _DEBUG
	Profiler::Get().BeginFrame();
	PROFILE_SCOPE("Engine BeginFrame");
#endif
	// すべてのゲームで必要になる毎フレーム処理を先に更新する
	frameRateController_->BeginFrame();
	time_->Update();
	input_->Update();
	audio_->Update();
	// 前フレーム以前にGPU利用が終わったテクスチャとSRVを再利用可能にする。
	textureManager_->CollectGarbage();
	if (!sceneInitialized_) {
		throw std::logic_error("SetInitialScene must be called from OnInitialize.");
	}
	// シーン切り替えは固定更新・通常更新のどちらも始まっていない境界で適用する。
	sceneManager_->BeginFrame();

#ifdef _DEBUG
	// デバッグUIのフレーム管理はゲームシーンの有無に依存させない
	imguiManager_->BeginFrame();
#endif

	// 蓄積時間が固定間隔を満たす間、物理・固定ロジックを一定刻みで進める
	while (time_->ConsumeFixedStep()) {
		OnFixedUpdate();
		sceneManager_->FixedUpdate();
		// 全GameObjectがColliderの位置を同期してから、世界全体を一度だけ判定する。
		collisionWorld_->Update();
	}
}

void EngineCore::OnInitialize() {}

void EngineCore::OnUpdate() {}

void EngineCore::OnFixedUpdate() {}

void EngineCore::OnDrawWorld() {}

void EngineCore::OnDrawUI() {}

void EngineCore::OnFinalize() {}

void EngineCore::SetInitialScene(std::unique_ptr<IScene> initialScene) {
	if (!initialized_ || sceneInitialized_) {
		throw std::logic_error("Initial scene can only be set once after engine initialization.");
	}
	SceneContext context{};
	context.audio = audio_.get();
	context.assets = assetManager_.get();
	context.collisionWorld = collisionWorld_.get();
	context.directXCommon = dxCommon_.get();
	context.input = input_.get();
	context.textureManager = textureManager_.get();
	context.modelManager = modelManager_.get();
	context.spriteCommon = spriteCommon_.get();
	context.object3dCommon = object3dCommon_.get();
	context.time = time_.get();
	context.frameRateController = frameRateController_.get();
	context.gpuParticlePipeline = gpuParticlePipeline_.get();
#ifdef _DEBUG
	context.imguiManager = imguiManager_.get();
#endif
	sceneManager_->Initialize(context, std::move(initialScene));
	sceneInitialized_ = true;
}

void EngineCore::SetSceneFactory(std::unique_ptr<AbstractSceneFactory> sceneFactory) {
	// SceneManager初期化後の差し替えは、実行中シーンとの対応が崩れるため許可しない。
	if (!initialized_ || sceneInitialized_) {
		throw std::logic_error("Scene factory can only be set after engine initialization and before the initial scene.");
	}

	sceneManager_->SetSceneFactory(std::move(sceneFactory));
}

void EngineCore::SetInitialScene(std::string_view initialSceneId) {
	if (!initialized_ || sceneInitialized_) {
		throw std::logic_error("Initial scene can only be set once after engine initialization.");
	}

	// 各シーンへ貸し出すサービスを一つのContextにまとめる。
	// 所有権はEngineCoreが保持し、シーンは実行期間中だけ参照する。
	SceneContext context{};
	context.audio = audio_.get();
	context.assets = assetManager_.get();
	context.collisionWorld = collisionWorld_.get();
	context.directXCommon = dxCommon_.get();
	context.input = input_.get();
	context.textureManager = textureManager_.get();
	context.modelManager = modelManager_.get();
	context.spriteCommon = spriteCommon_.get();
	context.object3dCommon = object3dCommon_.get();
	context.time = time_.get();
	context.frameRateController = frameRateController_.get();
	context.gpuParticlePipeline = gpuParticlePipeline_.get();
#ifdef _DEBUG
	context.imguiManager = imguiManager_.get();
#endif

	// Factoryによる生成と初期化はSceneManagerへ一任する。
	sceneManager_->Initialize(context, initialSceneId);
	sceneInitialized_ = true;
}

SceneManager &EngineCore::GetSceneManager() {
	if (!sceneInitialized_) {
		throw std::logic_error("SceneManager is not initialized.");
	}
	return *sceneManager_;
}

bool EngineCore::IsEndRequest() {
	return winApp_->ProcessMessage();
}

void EngineCore::BeginDraw() {
	// フレームのコマンド記録を開始した後、World専用RenderTextureへ描画先を切り替える。
	dxCommon_->PreDraw();
	renderTargetManager_->BeginWorldPass();
}

void EngineCore::BeginUIDraw() {
	// World結果を画面へコピーし、後続のSprite UIをポストエフェクト対象外にする。
	renderTargetManager_->CompositeToSwapChain();
}

void EngineCore::EndDraw() {
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

void EngineCore::Finalize() {
	initialized_ = false;
	if (sceneManager_) {
		sceneManager_->Finalize();
	}
	sceneInitialized_ = false;
	if (dxCommon_) {
		// 複数フレームを並行実行しているため、GPUリソースを所有する各Managerの破棄前に一度だけ同期する。
		dxCommon_->WaitForGPUIdle();
	}

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
	sceneManager_.reset();
	engineSettings_.reset();
	gpuParticlePipeline_.reset();
	object3dCommon_.reset();
	spriteCommon_.reset();
	modelManager_.reset();
	textureManager_.reset();
	renderTargetManager_.reset();
	srvManager_.reset();
	shaderCompiler_.reset();
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
