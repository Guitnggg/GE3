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
	if (initialized_) { throw std::logic_error("GameScene is already initialized."); }
	context_ = context;

	try {
		fanfareSound_ = context_.audio->Load("fanfare.wav");
		uvCheckerTexture_ = context_.textureManager->Load("resource/uvChecker.png");
		monsterBallTexture_ = context_.textureManager->Load("resource/monsterBall.png");

		sprite_ = std::make_unique<Sprite>();
		sprite_->Initialize(context_.spriteCommon, context_.textureManager, uvCheckerTexture_);

		object3d_ = std::make_unique<Object3d>();
		object3d_->Initialize(context_.object3dCommon, context_.textureManager, context_.modelManager->Load());
		camera_ = std::make_unique<Camera>();
		camera_->Update();

		sphere_ = std::make_unique<Object3d>();
		sphere_->Initialize(context_.object3dCommon, context_.textureManager,
			context_.modelManager->Create(MeshGenerator::CreateSphere(kSphereSubdivisions), uvCheckerTexture_));
		initialized_ = true;
	} catch (...) {
		Finalize();
		throw;
	}
}

void GameScene::Update() {
#ifdef _DEBUG
	context_.imguiManager->BeginFrame();
	context_.imguiManager->DrawDebugWindow(isModel_, isSphere_, isRotate_, isSprite_, textureChange_,
		*sphere_->GetMaterialData(), sphere_->GetTransform(), *sphere_->GetDirectionalLightData(), sprite_->GetTransform(),
		sprite_->GetUvTransform(), *context_.audio, fanfareSound_, *context_.frameRateController, *context_.time);
#endif

	if (context_.input->TriggerKey(DIK_0)) { context_.audio->Play(fanfareSound_, false, 1.0f, 1.0f); }
	constexpr float kSphereRotationSpeed = 3.0f;
	if (isRotate_) { sphere_->GetTransform().rotate.y -= kSphereRotationSpeed * context_.time->GetDeltaTime(); }

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
	context_.object3dCommon->CommonDrawSetting();
	if (isSphere_) {
		sphere_->SetTextureIndex(textureChange_ ? monsterBallTexture_ : uvCheckerTexture_);
		sphere_->Draw();
	}
	if (isModel_) { object3d_->Draw(); }
	if (isSprite_) {
		sprite_->SetTextureIndex(textureChange_ ? monsterBallTexture_ : uvCheckerTexture_);
		sprite_->Draw();
	}
}

void GameScene::Finalize() {
	initialized_ = false;
	sprite_.reset();
	object3d_.reset();
	sphere_.reset();
	camera_.reset();
	fanfareSound_ = Audio::kInvalidSoundHandle;
	uvCheckerTexture_ = 0;
	monsterBallTexture_ = 0;
	context_ = {};
}
