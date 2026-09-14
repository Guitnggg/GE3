#include "application/scenes/GameScene.h"

#include "engine/2D/Sprite.h"
#include "engine/3D/camera/Camera.h"
#include "engine/3D/model/MeshGenerator.h"
#include "engine/3D/model/ModelManager.h"
#include "engine/3D/object/Object3d.h"
#include "engine/3D/object/Object3dCommon.h"
#include "engine/core/WinApp.h"
#include "engine/core/timing/FrameRateController.h"
#include "engine/core/timing/Time.h"
#include "engine/graphics/debug/ImGuiManager.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/input/Input.h"

#include <stdexcept>

GameScene::GameScene() = default;
GameScene::~GameScene() { Finalize(); }

void GameScene::Initialize(const SceneContext& context) {
	// 多重初期化を防ぎ、Frameworkが所有する共通機能への参照を保持する
	if (initialized_) { throw std::logic_error("GameScene is already initialized."); }
	context_ = context;

	try {
		// シーンで使用する音声とテクスチャを先に読み込む
		fanfareSound_ = context_.audio->Load("fanfare.wav");
		uvCheckerTexture_ = context_.textureManager->Load("resource/uvChecker.png");
		monsterBallTexture_ = context_.textureManager->Load("resource/monsterBall.png");

		// 2Dスプライトを生成し、既定テクスチャを設定する
		sprite_ = std::make_unique<Sprite>();
		sprite_->Initialize(context_.spriteCommon, context_.textureManager, uvCheckerTexture_);

		// ファイルから読み込むモデルと、それを映すカメラを生成する
		object3d_ = std::make_unique<Object3d>();
		object3d_->Initialize(context_.object3dCommon, context_.textureManager, context_.modelManager->Load());
		camera_ = std::make_unique<Camera>();
		camera_->Update();

		// 手続き生成した球メッシュから、別の3Dオブジェクトを生成する
		sphere_ = std::make_unique<Object3d>();
		sphere_->Initialize(context_.object3dCommon, context_.textureManager,
			context_.modelManager->Create(MeshGenerator::CreateSphere(kSphereSubdivisions), uvCheckerTexture_));
		initialized_ = true;
	} catch (...) {
		// 初期化途中で失敗した場合も、生成済みの要素を確実に解放する
		Finalize();
		throw;
	}
}

void GameScene::Update() {
#ifdef _DEBUG
	// デバッグビルドでは、各表示設定を編集するUIを構築する
	context_.imguiManager->BeginFrame();
	context_.imguiManager->DrawDebugWindow(isModel_, isSphere_, isRotate_, isSprite_, textureChange_,
		*sphere_->GetMaterialData(), sphere_->GetTransform(), *sphere_->GetDirectionalLightData(), sprite_->GetTransform(),
		sprite_->GetUvTransform(), *context_.audio, fanfareSound_, *context_.frameRateController, *context_.time);
#endif

	// キー入力による効果音再生と、経過時間に依存しない速度で球を回転させる
	if (context_.input->TriggerKey(DIK_0)) { context_.audio->Play(fanfareSound_, false, 1.0f, 1.0f); }
	constexpr float kSphereRotationSpeed = 3.0f;
	if (isRotate_) { sphere_->GetTransform().rotate.y -= kSphereRotationSpeed * context_.time->GetDeltaTime(); }

	// カメラを先に更新し、その最新行列を各描画オブジェクトへ反映する
	camera_->Update();
	object3d_->Update(*camera_);
	sphere_->Update(*camera_);
	sprite_->Update(static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight));

#ifdef _DEBUG
	context_.imguiManager->EndFrame();
#endif
}

void GameScene::FixedUpdate() {
	// 将来、当たり判定や物理処理を固定時間間隔で実行する。
}

void GameScene::Draw() {
	// 3D共通パイプラインを設定してから、有効なオブジェクトだけを描画する
	context_.object3dCommon->CommonDrawSetting();
	if (isSphere_) {
		sphere_->SetTextureIndex(textureChange_ ? monsterBallTexture_ : uvCheckerTexture_);
		sphere_->Draw();
	}
	if (isModel_) { object3d_->Draw(); }
	if (isSprite_) {
		// Sprite::Drawが専用の2Dパイプラインへ切り替えて描画する
		sprite_->SetTextureIndex(textureChange_ ? monsterBallTexture_ : uvCheckerTexture_);
		sprite_->Draw();
	}
}

void GameScene::Finalize() {
	initialized_ = false;

	// シーン所有オブジェクトを解放し、外部機能への非所有参照を破棄する
	sprite_.reset();
	object3d_.reset();
	sphere_.reset();
	camera_.reset();
	fanfareSound_ = Audio::kInvalidSoundHandle;
	uvCheckerTexture_ = 0;
	monsterBallTexture_ = 0;
	context_ = {};
}
