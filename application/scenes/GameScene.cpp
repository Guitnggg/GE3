#include "application/scenes/GameScene.h"

#include "engine/3D/model/MeshGenerator.h"
#include "engine/3D/model/ModelManager.h"
#include "engine/3D/object/Object3dCommon.h"
#include "engine/collision/CollisionWorld.h"
#include "engine/core/timing/Time.h"
#include "engine/graphics/debug/ImGuiManager.h"
#include "engine/graphics/resource/TextureManager.h"
#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#endif

#include <stdexcept>

namespace {
constexpr uint32_t kSphereSubdivisions = 12;
}

GameScene::~GameScene() {
	Finalize();
}

void GameScene::Initialize(const SceneContext &context) {
	if (initialized_) {
		throw std::logic_error("GameScene is already initialized.");
	}
	context_ = context;
	try {
		InitializeInputActions();
		InitializeGameObjects();
#ifdef _DEBUG
		parameterEditor_.Initialize(context_.audio);
#endif
		initialized_ = true;
		ResetGame();
	} catch (...) {
		Finalize();
		throw;
	}
}

void GameScene::InitializeInputActions() {
	moveAction_ = context_.input->AddAxis2DAction("Move");
	context_.input->BindKeyboardAxis2D(moveAction_, DIK_W, DIK_S, DIK_A, DIK_D);
	context_.input->BindLeftStick(moveAction_);
	shootAction_ = context_.input->AddButtonAction("Shoot");
	context_.input->BindMouseButton(shootAction_, 0);
	context_.input->BindPadButton(shootAction_, XINPUT_GAMEPAD_RIGHT_SHOULDER);
	lockOnAction_ = context_.input->AddButtonAction("LockOn");
	context_.input->BindMouseButton(lockOnAction_, 1);
	context_.input->BindPadButton(lockOnAction_, XINPUT_GAMEPAD_LEFT_SHOULDER);
	restartAction_ = context_.input->AddButtonAction("Restart");
	context_.input->BindKey(restartAction_, DIK_R);
	context_.input->BindPadButton(restartAction_, XINPUT_GAMEPAD_START);
}

void GameScene::InitializeGameObjects() {
	texture_ = context_.textureManager->Load("resource/textures/monsterBall.png");
	lockOnTexture_ = context_.textureManager->Load("resource/textures/Lockon.png");
	particleTexture_ = context_.textureManager->Load("resource/textures/particleSoft.png");
	sphereModel_ = context_.modelManager->Create(MeshGenerator::CreateSphere(kSphereSubdivisions), texture_);
	playerModel_ = context_.modelManager->Load("resource/models/player", "player.obj");
	missileModel_ = context_.modelManager->Load("resource/models/missile", "missile.obj");
	auto mapModel = context_.modelManager->Load("resource/models/building", "map.obj");

	stageEnvironment_ = std::make_unique<StageEnvironment>();
	stageEnvironment_->Initialize(context_.object3dCommon, context_.textureManager, mapModel);
	player_ = std::make_unique<Player>();
	player_->Initialize(context_.spriteCommon,
	                    context_.object3dCommon,
	                    context_.textureManager,
	                    context_.input,
	                    playerModel_,
	                    texture_,
	                    moveAction_,
	                    shootAction_);

	playerEngineEffect_ = std::make_unique<PlayerEngineEffect>();
	playerEngineEffect_->Initialize(context_.directXCommon,
	                                context_.gpuParticlePipeline,
	                                context_.textureManager,
	                                particleTexture_,
	                                parameters_.engineParticle);
	enemyDeathEffect_ = std::make_unique<EnemyDeathEffect>();
	enemyDeathEffect_->Initialize(
	    context_.directXCommon, context_.gpuParticlePipeline, context_.textureManager, particleTexture_);
	missileTrailEffect_ = std::make_unique<MissileTrailEffect>();
	missileTrailEffect_->Initialize(
	    context_.directXCommon, context_.gpuParticlePipeline, context_.textureManager, particleTexture_);

	enemyManager_.Initialize(context_.collisionWorld,
	                         context_.spriteCommon,
	                         context_.object3dCommon,
	                         context_.textureManager,
	                         sphereModel_,
	                         lockOnTexture_);
	weaponManager_.Initialize(context_.collisionWorld,
	                          context_.object3dCommon,
	                          context_.textureManager,
	                          context_.input,
	                          sphereModel_,
	                          missileModel_,
	                          lockOnAction_);
}

void GameScene::ResetGame() {
	enemyManager_.Reset();
	weaponManager_.Reset(enemyManager_);
	player_->Reset(parameters_.startingLives);
	stageEnvironment_->Reset();
	playerEngineEffect_->Reset(parameters_.engineParticle);
	enemyDeathEffect_->Reset();
	missileTrailEffect_->Reset();
	parameterEditor_.SetPaused(false);
	score_ = 0;
	gameOver_ = false;
}

void GameScene::Update() {
	const float deltaTime = context_.time->GetDeltaTime();
	if (!gameOver_ && !parameterEditor_.IsPaused()) {
		bool acceptFireInput = true;
#ifdef _DEBUG
		acceptFireInput = !ImGui::GetIO().WantCaptureMouse;
#endif
		UpdateGameplay(deltaTime, acceptFireInput);
	} else if (gameOver_ && context_.input->TriggerAction(restartAction_)) {
		ResetGame();
	}

	const float simulationDeltaTime = (!gameOver_ && !parameterEditor_.IsPaused()) ? deltaTime : 0.0f;
	UpdateFrameSystems(simulationDeltaTime);
	DrawDebugUi();
}

void GameScene::UpdateGameplay(float deltaTime, bool acceptFireInput) {
	if (player_->Update(deltaTime, parameters_.railSpeed, parameters_.playerMoveSpeed, acceptFireInput)) {
		weaponManager_.Shoot(player_->GetShotOrigin(), player_->GetShotDirection());
	}
	weaponManager_.UpdateLockOn(
	    deltaTime, acceptFireInput, player_->GetShotDirection(), player_->GetPosition(), enemyManager_);

	for (const uint64_t id : enemyManager_.RemovePassed(player_->GetPosition().z)) {
		weaponManager_.OnEnemyRemoved(id);
		player_->Damage();
	}
	gameOver_ = player_->IsDead();
	if (gameOver_) {
		weaponManager_.ClearLockOn(enemyManager_);
		return;
	}
	if (parameters_.enemySpawningEnabled) {
		enemyManager_.UpdateSpawning(deltaTime, player_->GetCameraZ(), parameters_, score_);
	}
}

void GameScene::UpdateFrameSystems(float deltaTime) {
	// 停止中も行列は更新し、現在の画面をそのまま描画できるようにする。
	stageEnvironment_->Update(player_->GetCamera(), player_->GetCameraZ());
	enemyManager_.Update(player_->GetCamera(), deltaTime, parameters_.enemyRotationSpeed);
	weaponManager_.UpdateProjectiles(player_->GetCamera(), deltaTime, enemyManager_);
	missileTrailEffect_->EmitTrails(weaponManager_.GetMissilePositions(), deltaTime);
	missileTrailEffect_->Update(deltaTime);
	playerEngineEffect_->Update(deltaTime, player_->GetPosition(), parameters_.engineParticle);

	context_.collisionWorld->Update();
	const std::vector<Vector3> destroyedPositions = weaponManager_.ResolveProjectileHits(enemyManager_);
	for (const Vector3 &position : destroyedPositions) {
		enemyDeathEffect_->Emit(position);
	}
	score_ += static_cast<uint32_t>(destroyedPositions.size());
	enemyDeathEffect_->Update(deltaTime);
}

void GameScene::DrawDebugUi() {
#ifdef _DEBUG
	context_.imguiManager->BeginFrame();
	if (parameterEditor_.Draw(parameters_, *context_.frameRateController, *context_.time)) {
		ResetGame();
		stageEnvironment_->Update(player_->GetCamera(), player_->GetCameraZ());
	}
	ImGui::SetNextWindowPos({12.0f, 12.0f}, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.72f);
	ImGui::Begin("3D RAIL SHOOTER",
	             nullptr,
	             ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::Text("SCORE  %u", score_);
	ImGui::Text("LIVES  %u", player_->GetLives());
	ImGui::TextUnformatted("MOVE: WASD    AIM: Mouse    NORMAL: Left Click");
	ImGui::TextUnformatted("LOCK: Hold Right Click    MISSILE: Release Right Click");
	if (weaponManager_.HasLock()) {
		ImGui::TextColored({1.0f, 0.85f, 0.1f, 1.0f}, "LOCKED  %zu / 5", weaponManager_.GetLockCount());
	}
	if (parameterEditor_.IsPaused()) {
		ImGui::TextColored({1.0f, 0.8f, 0.2f, 1.0f}, "PAUSED");
	}
	if (gameOver_) {
		ImGui::Separator();
		ImGui::TextColored({1.0f, 0.25f, 0.2f, 1.0f}, "GAME OVER");
		ImGui::TextUnformatted("Press R to restart");
	}
	ImGui::End();
	context_.imguiManager->EndFrame();
#endif
}

void GameScene::FixedUpdate() {}

void GameScene::Draw() {
	context_.object3dCommon->CommonDrawSetting();
	stageEnvironment_->Draw();
	player_->DrawShip();
	enemyManager_.Draw();
	weaponManager_.Draw();
	missileTrailEffect_->Draw(player_->GetCamera());
	playerEngineEffect_->Draw(player_->GetCamera());
	enemyDeathEffect_->Draw(player_->GetCamera());
	enemyManager_.DrawLockOnMarkers();
	player_->DrawReticle();
}

void GameScene::Finalize() {
	initialized_ = false;
	parameterEditor_.Finalize();
	weaponManager_.Reset(enemyManager_);
	enemyManager_.Reset();
	missileTrailEffect_.reset();
	enemyDeathEffect_.reset();
	playerEngineEffect_.reset();
	if (stageEnvironment_) {
		stageEnvironment_->Finalize();
	}
	stageEnvironment_.reset();
	player_.reset();
	missileModel_.reset();
	playerModel_.reset();
	sphereModel_.reset();
	texture_ = lockOnTexture_ = particleTexture_ = 0;
	moveAction_ = shootAction_ = lockOnAction_ = restartAction_ = kInvalidInputActionId;
	context_ = {};
}
