#include "application/scenes/GameScene.h"

#include "application/characters/Player.h"
#include "engine/3D/model/MeshGenerator.h"
#include "engine/3D/model/ModelManager.h"
#include "engine/3D/object/Object3dCommon.h"
#include "engine/core/timing/Time.h"
#include "engine/graphics/debug/ImGuiManager.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/input/Input.h"
#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#endif
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
		// 敵と弾で共有するメッシュを1度だけ生成する
		texture_ = context_.textureManager->Load("resource/textures/monsterBall.png");
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
			context_.textureManager, context_.input, playerModel_, texture_);
		enemyManager_.Initialize(context_.object3dCommon, context_.textureManager, sphereModel_);
		weaponManager_.Initialize(context_.object3dCommon, context_.textureManager, context_.input,
			sphereModel_, missileModel_);
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
			weaponManager_.UpdateLockOn(deltaTime, acceptFireInput, player_->GetShotOrigin(),
				player_->GetShotDirection(), player_->GetPosition(), enemyManager_);
			const std::vector<uint64_t> passedIds = enemyManager_.RemovePassed(player_->GetPosition().z);
			for (const uint64_t id : passedIds) {
				weaponManager_.OnEnemyRemoved(id);
				player_->Damage();
			}
			gameOver_ = player_->IsDead();
			if (gameOver_) { weaponManager_.ClearLockOn(enemyManager_); }
			if (!gameOver_) {
				enemyManager_.UpdateSpawning(deltaTime, player_->GetCameraZ(), parameters_, score_);
			}
		}
	} else if (context_.input->TriggerKey(DIK_R)) { ResetGame(); }
	// 停止中も描画行列は更新し、現在の画面をそのまま表示できるようにする
	UpdateEnvironment();
	const float enemyDeltaTime = (!gameOver_ && !parameterEditor_.IsPaused()) ? deltaTime : 0.0f;
	enemyManager_.Update(player_->GetCamera(), enemyDeltaTime, parameters_.enemyRotationSpeed);
	score_ += weaponManager_.UpdateProjectiles(player_->GetCamera(), enemyDeltaTime, enemyManager_);

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
	if (weaponManager_.HasLock()) { ImGui::TextColored({1.0f, 0.85f, 0.1f, 1.0f}, "LOCKED"); }
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
	player_->DrawReticle();
}

void GameScene::Finalize() {
	// シーン所有物を解放してから、Frameworkへの非所有参照を破棄する
	initialized_ = false;
	parameterEditor_.Finalize();
	weaponManager_.Reset(enemyManager_); enemyManager_.Reset(); mapSegments_.clear(); player_.reset();
	mapModel_.reset(); missileModel_.reset(); playerModel_.reset(); sphereModel_.reset();
	texture_ = 0; context_ = {};
}
