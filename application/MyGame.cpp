#include "application/MyGame.h"

#include "engine/2d/Sprite.h"
#include "engine/3d/Camera.h"
#include "engine/3d/MeshGenerator.h"
#include "engine/3d/ModelManager.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/TextureManager.h"
#include "engine/core/FrameRateController.h"
#include "engine/core/ImGuiManager.h"
#include "engine/core/Input.h"
#include "engine/core/Time.h"
#include "engine/core/WinApp.h"

#include <stdexcept>

MyGame::MyGame() = default;

MyGame::~MyGame() { Finalize(); }

void MyGame::Initialize() {
	if (initialized_ || sprite_ || object3d_ || sphere_ || camera_) {
		throw std::logic_error("MyGame is already initialized or partially initialized.");
	}

	try {
		// 最初にゲーム共通機能を初期化する
		Framework::Initialize();

	// このゲームで使用する音声を読み込む
	fanfareSound_ = audio_->Load("fanfare.wav");

	// 描画で切り替えて使用するテクスチャを読み込む
	uvCheckerTexture_ = textureManager_->Load("resource/uvChecker.png");
	monsterBallTexture_ = textureManager_->Load("resource/monsterBall.png");

	// スプライト自身へ描画用テクスチャと各GPUリソースを持たせる
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(spriteCommon_.get(), textureManager_.get(), uvCheckerTexture_);

	// 3Dオブジェクトとカメラの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon_.get(), textureManager_.get(), modelManager_->Load());
	camera_ = std::make_unique<Camera>();
	camera_->Update();

	// 手続き生成した頂点列も、OBJと同じObject3d経由で管理する
	sphere_ = std::make_unique<Object3d>();
	sphere_->Initialize(object3dCommon_.get(), textureManager_.get(),
		modelManager_->Create(MeshGenerator::CreateSphere(kSphereSubdivisions), uvCheckerTexture_));

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
		*sphere_->GetMaterialData(), sphere_->GetTransform(), *sphere_->GetDirectionalLightData(), sprite_->GetTransform(),
		sprite_->GetUvTransform(), *audio_, fanfareSound_, *frameRateController_, *time_);
#endif

	// 入力と音声など、ゲーム共通の毎フレーム処理
	Framework::Update();

	// ここからこのゲーム固有の更新処理
	if (input_->TriggerKey(DIK_0)) { audio_->Play(fanfareSound_, false, 1.0f, 1.0f); }
	constexpr float kSphereRotationSpeed = 3.0f; // radians per second
	if (isRotate_) {
		sphere_->GetTransform().rotate.y -= kSphereRotationSpeed * time_->GetDeltaTime();
	}

	// カメラを先に更新し、各3Dオブジェクトへ最新のViewProjectionを渡す
	camera_->Update();
	object3d_->Update(*camera_);
	sphere_->Update(*camera_);

	// 画面サイズを渡し、スプライト内部で座標行列とUV行列を更新する
	sprite_->Update(static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight));
#ifdef _DEBUG
	imguiManager_->EndFrame();
#endif
}

void MyGame::FixedUpdate() {
	// 当たり判定や物理移動は、必要になった時点でここへ実装する。
	// 1ステップの秒数にはtime_->GetFixedDeltaTime()を使用する。
}

void MyGame::Draw() {
	// Framework側でフレームの描画準備を行う
	BeginDraw();
	object3dCommon_->CommonDrawSetting();

	// 球体を描画
	if (isSphere_) {
		sphere_->SetTextureIndex(textureChange_ ? monsterBallTexture_ : uvCheckerTexture_);
		sphere_->Draw();
	}

	// OBJモデルを描画
	if (isModel_) { object3d_->Draw(); }

	// 2Dスプライトを描画
	if (isSprite_) {
		sprite_->SetTextureIndex(textureChange_ ? monsterBallTexture_ : uvCheckerTexture_);
		sprite_->Draw();
	}
	// ImGuiの描画と画面表示はFramework側で行う
	EndDraw();
}

void MyGame::Finalize() {
	initialized_ = false;

	// このゲーム固有のオブジェクトを先に解放する
	sprite_.reset();
	object3d_.reset();
	sphere_.reset();
	camera_.reset();

	fanfareSound_ = Audio::kInvalidSoundHandle;
	uvCheckerTexture_ = 0;
	monsterBallTexture_ = 0;

	// 最後にゲーム共通機能を解放する
	Framework::Finalize();
}
