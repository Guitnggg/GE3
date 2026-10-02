#include "application/scenes/GameScene.h"

#include "application/characters/Player.h"
#include "engine/3D/model/MeshGenerator.h"
#include "engine/3D/model/ModelManager.h"
#include "engine/3D/object/Object3dCommon.h"
#include "engine/core/timing/Time.h"
#include "engine/graphics/debug/ImGuiManager.h"
#include "engine/collision/CollisionWorld.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/effects/particle/GPUParticleEmitter.h"
#include "engine/effects/particle/GPUParticleSystem.h"
#include "engine/input/Input.h"
#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#endif
#include <algorithm>
#include <stdexcept>

namespace {
constexpr uint32_t kSphereSubdivisions = 12;
constexpr uint32_t kMapSegmentCount = 7;
constexpr float kMapScale = 2.0f;
constexpr float kMapSegmentLength = 22.0f;
constexpr float kMapStartZ = -5.0f;
}

GameScene::~GameScene() { Finalize(); }

void GameScene::Initialize(const SceneContext& context) {
	// 多重初期化を防ぎ、Frameworkが所有する共通機能への参照を保持する
	if (initialized_) { throw std::logic_error("GameScene is already initialized."); }
	context_ = context;
	try {
		// ゲームコードから物理デバイスを隠し、同じ操作へ複数デバイスを割り当てる
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
		// 敵と弾で共有するメッシュを1度だけ生成する
		texture_ = context_.textureManager->Load("resource/textures/monsterBall.png");
		lockOnTexture_ = context_.textureManager->Load("resource/textures/Lockon.png");
		particleTexture_ = context_.textureManager->Load("resource/textures/particleSoft.png");
		sphereModel_ = context_.modelManager->Create(MeshGenerator::CreateSphere(kSphereSubdivisions), texture_);
		playerModel_ = context_.modelManager->Load("resource/models/player", "player.obj");
		missileModel_ = context_.modelManager->Load("resource/models/missile", "missile.obj");
		mapModel_ = context_.modelManager->Load("resource/models/building", "map.obj");
		for (uint32_t i = 0; i < kMapSegmentCount; ++i) {
			auto segment = std::make_unique<Object3d>();
			segment->Initialize(context_.object3dCommon, context_.textureManager, mapModel_);
			segment->GetTransform().scale = {kMapScale, kMapScale, kMapScale};
			segment->GetTransform().translate = {0.0f, -4.0f, kMapStartZ + kMapSegmentLength * i};
			segment->GetMaterialData()->color = {0.38f, 0.48f, 0.68f, 1.0f};
			segment->GetDirectionalLightData()->direction = {-0.35f, -1.0f, 0.25f};
			segment->GetDirectionalLightData()->intensity = 1.25f;
			mapSegments_.push_back(std::move(segment));
		}
		// プレイヤーとゲーム要素を生成する
		player_ = std::make_unique<Player>();
		player_->Initialize(context_.spriteCommon, context_.object3dCommon,
			context_.textureManager, context_.input, playerModel_, texture_, moveAction_, shootAction_);
		// 1つのGPUプールと中央噴射口を作り、Presetで噴射の見た目を定義する。
		engineParticleSystem_ = std::make_unique<GPUParticleSystem>();
		engineParticleSystem_->Initialize(context_.directXCommon, context_.textureManager, particleTexture_, 4096);
		GPUParticleEmitData engineEmit{};
		engineEmit.seed = 0x454e474eu;
		engineEmitter_ = std::make_unique<GPUParticleEmitter>();
		engineEmitter_->Initialize(engineParticleSystem_.get(), engineEmit, parameters_.engineParticleInterval);
		enemyManager_.Initialize(context_.collisionWorld, context_.spriteCommon,
			context_.object3dCommon, context_.textureManager, sphereModel_, lockOnTexture_);
		weaponManager_.Initialize(context_.collisionWorld, context_.object3dCommon, context_.textureManager, context_.input,
			sphereModel_, missileModel_, lockOnAction_);
#ifdef _DEBUG
		parameterEditor_.Initialize(context_.audio);
#endif
		initialized_ = true;
		ResetGame();
	} catch (...) { Finalize(); throw; }
}

void GameScene::ResetGame() {
	// 動的な敵を破棄し、プレイヤーとゲーム進行用の値をまとめて初期化する
	enemyManager_.Reset();
	weaponManager_.Reset(enemyManager_);
	player_->Reset(parameters_.startingLives);
	engineParticleSystem_->Reset();
	engineEmitter_->Reset();
	engineEmitter_->SetActive(parameters_.engineParticleEnabled);
	parameterEditor_.SetPaused(false);
	score_ = 0;
	gameOver_ = false;
	for (size_t i = 0; i < mapSegments_.size(); ++i) {
		mapSegments_[i]->GetTransform().translate.z = kMapStartZ + kMapSegmentLength * i;
	}
}

void GameScene::UpdateEnvironment() {
	const float loopLength = kMapSegmentLength * static_cast<float>(mapSegments_.size());
	for (auto& segment : mapSegments_) {
		// カメラ後方へ完全に抜けた街区を列の先頭へ送り、継ぎ目なく街並みを続ける
		if (segment->GetTransform().translate.z < player_->GetCameraZ() - kMapSegmentLength) {
			segment->GetTransform().translate.z += loopLength;
		}
		segment->Update(player_->GetCamera());
	}
}

void GameScene::Update() {
	const float deltaTime = context_.time->GetDeltaTime();
	// 一時停止中とゲームオーバー中はゲームロジックを進めない
	if (!gameOver_) {
		if (!parameterEditor_.IsPaused()) {
			bool acceptFireInput = true;
#ifdef _DEBUG
			// ImGui操作中も表示位置は同期し、射撃だけを抑制する
			acceptFireInput = !ImGui::GetIO().WantCaptureMouse;
#endif
			if (player_->Update(deltaTime, parameters_.railSpeed, parameters_.playerMoveSpeed, acceptFireInput)) {
				weaponManager_.Shoot(player_->GetShotOrigin(), player_->GetShotDirection());
			}
			weaponManager_.UpdateLockOn(deltaTime, acceptFireInput,
				player_->GetShotDirection(), player_->GetPosition(), enemyManager_);
			const std::vector<uint64_t> passedIds = enemyManager_.RemovePassed(player_->GetPosition().z);
			for (const uint64_t id : passedIds) {
				weaponManager_.OnEnemyRemoved(id);
				player_->Damage();
			}
			gameOver_ = player_->IsDead();
			if (gameOver_) { weaponManager_.ClearLockOn(enemyManager_); }
			if (!gameOver_ && parameters_.enemySpawningEnabled) {
				enemyManager_.UpdateSpawning(deltaTime, player_->GetCameraZ(), parameters_, score_);
			}
		}
	} else if (context_.input->TriggerAction(restartAction_)) { ResetGame(); }
	// 停止中も描画行列は更新し、現在の画面をそのまま表示できるようにする
	UpdateEnvironment();
	const float enemyDeltaTime = (!gameOver_ && !parameterEditor_.IsPaused()) ? deltaTime : 0.0f;
	enemyManager_.Update(player_->GetCamera(), enemyDeltaTime, parameters_.enemyRotationSpeed);
	weaponManager_.UpdateProjectiles(player_->GetCamera(), enemyDeltaTime, enemyManager_);
	UpdateEngineParticles(enemyDeltaTime);
	context_.collisionWorld->Update();
	score_ += weaponManager_.ResolveProjectileHits(enemyManager_);

#ifdef _DEBUG
	// ゲームHUDとパラメータエディタは同じImGuiフレーム内へ構築する
	context_.imguiManager->BeginFrame();
	if (parameterEditor_.Draw(parameters_, *context_.frameRateController, *context_.time)) {
		ResetGame();
		UpdateEnvironment();
	}
	ImGui::SetNextWindowPos({12.0f, 12.0f}, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.72f);
	ImGui::Begin("3D RAIL SHOOTER", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::Text("SCORE  %u", score_);
	ImGui::Text("LIVES  %u", player_->GetLives());
	ImGui::TextUnformatted("MOVE: WASD    AIM: Mouse    NORMAL: Left Click");
	ImGui::TextUnformatted("LOCK: Hold Right Click    MISSILE: Release Right Click");
	if (weaponManager_.HasLock()) {
		ImGui::TextColored({1.0f, 0.85f, 0.1f, 1.0f}, "LOCKED  %zu / 5", weaponManager_.GetLockCount());
	}
	if (parameterEditor_.IsPaused()) { ImGui::TextColored({1.0f, 0.8f, 0.2f, 1.0f}, "PAUSED"); }
	if (gameOver_) { ImGui::Separator(); ImGui::TextColored({1.0f, 0.25f, 0.2f, 1.0f}, "GAME OVER"); ImGui::TextUnformatted("Press R to restart"); }
	ImGui::End();
	context_.imguiManager->EndFrame();
#endif
}

// 現在のゲームロジックは可変時間更新で完結しているため固定更新は使用しない
void GameScene::FixedUpdate() {}

void GameScene::Draw() {
	// 3Dオブジェクトを先に描画し、最後に2D照準を前面へ重ねる
	context_.object3dCommon->CommonDrawSetting();
	for (const auto& segment : mapSegments_) { segment->Draw(); }
	player_->DrawShip();
	enemyManager_.Draw();
	weaponManager_.Draw();
	// 不透明オブジェクトの後に半透明・加算パーティクルを描画する。
	engineParticleSystem_->Draw(player_->GetCamera());
	enemyManager_.DrawLockOnMarkers();
	player_->DrawReticle();
}

void GameScene::Finalize() {
	// シーン所有物を解放してから、Frameworkへの非所有参照を破棄する
	initialized_ = false;
	parameterEditor_.Finalize();
	weaponManager_.Reset(enemyManager_); enemyManager_.Reset(); mapSegments_.clear();
	engineEmitter_.reset(); engineParticleSystem_.reset(); player_.reset();
	mapModel_.reset(); missileModel_.reset(); playerModel_.reset(); sphereModel_.reset();
	texture_ = 0; lockOnTexture_ = 0; particleTexture_ = 0;
	moveAction_ = shootAction_ = lockOnAction_ = restartAction_ = kInvalidInputActionId;
	context_ = {};
}

void GameScene::UpdateEngineParticles(float deltaTime) {
	const float minLifetime = std::min(parameters_.engineParticleMinLifetime, parameters_.engineParticleMaxLifetime);
	const float maxLifetime = std::max(parameters_.engineParticleMinLifetime, parameters_.engineParticleMaxLifetime);
	const float minSpeed = std::min(parameters_.engineParticleMinSpeed, parameters_.engineParticleMaxSpeed);
	const float maxSpeed = std::max(parameters_.engineParticleMinSpeed, parameters_.engineParticleMaxSpeed);

	GPUParticlePreset preset{};
	preset.acceleration = {0.0f, 0.0f, parameters_.engineParticleAccelerationZ};
	preset.drag = parameters_.engineParticleDrag;
	preset.startColor = parameters_.engineParticleStartColor;
	preset.endColor = parameters_.engineParticleEndColor;
	preset.startSize = {parameters_.engineParticleStartSize, parameters_.engineParticleStartSize};
	preset.endSize = {parameters_.engineParticleEndSize, parameters_.engineParticleEndSize};
	preset.minLifetime = minLifetime;
	preset.maxLifetime = maxLifetime;
	preset.blendMode = GPUParticleBlendMode::Additive;
	engineParticleSystem_->SetPreset(preset);

	GPUParticleEmitData emit{};
	emit.count = parameters_.engineParticleCount;
	emit.minVelocity = {-parameters_.engineParticleVelocitySpread, -parameters_.engineParticleVelocitySpread, -maxSpeed};
	emit.maxVelocity = {parameters_.engineParticleVelocitySpread, parameters_.engineParticleVelocitySpread, -minSpeed};
	emit.positionSpread = {parameters_.engineParticlePositionSpread,
		parameters_.engineParticlePositionSpread, parameters_.engineParticlePositionSpread};
	emit.seed = 0x454e474eu;
	engineEmitter_->SetEmitTemplate(emit);
	engineEmitter_->SetInterval(parameters_.engineParticleInterval);
	engineEmitter_->SetActive(parameters_.engineParticleEnabled);

	// プレイヤーモデルはローカル+Zが前方なので、-Z側の中央エンジン位置から噴射する。
	const Vector3& position = player_->GetPosition();
	engineEmitter_->Update(deltaTime, {position.x + parameters_.engineParticleOffsetX,
		position.y + parameters_.engineParticleOffsetY, position.z + parameters_.engineParticleOffsetZ});
	engineParticleSystem_->Update(deltaTime);
}
