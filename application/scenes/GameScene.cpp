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
}

GameScene::~GameScene() { Finalize(); }

void GameScene::Initialize(const SceneContext& context) {
	// 多重初期化を防ぎ、Frameworkが所有する共通機能への参照を保持する
	if (initialized_) { throw std::logic_error("GameScene is already initialized."); }
	context_ = context;
	try {
		// 敵とレールで共有するメッシュを1度だけ生成する
		texture_ = context_.textureManager->Load("resource/textures/monsterBall.png");
		sphereModel_ = context_.modelManager->Create(MeshGenerator::CreateSphere(kSphereSubdivisions), texture_);
		playerModel_ = context_.modelManager->Load("resource/models/player", "player.obj");
		missileModel_ = context_.modelManager->Load("resource/models/missile", "missile.obj");
		// プレイヤーを生成した後、進行方向の左右へレールマーカーを並べる
		player_ = std::make_unique<Player>();
		player_->Initialize(context_.spriteCommon, context_.object3dCommon,
			context_.textureManager, context_.input, playerModel_, texture_);
		enemyManager_.Initialize(context_.object3dCommon, context_.textureManager, sphereModel_);
		weaponManager_.Initialize(context_.object3dCommon, context_.textureManager, context_.input,
			sphereModel_, missileModel_);
		for (uint32_t i = 0; i < 24; ++i) {
			const float side = (i % 2 == 0) ? -6.5f : 6.5f;
			railMarkers_.push_back(CreateRailMarker(side, static_cast<float>(i / 2) * 7.0f));
		}
		initialized_ = true;
		ResetGame();
	} catch (...) { Finalize(); throw; }
}

std::unique_ptr<Object3d> GameScene::CreateRailMarker(float x, float z) {
	// 小さな青い球を等間隔に置き、前進していることが分かる目印にする
	auto marker = std::make_unique<Object3d>();
	marker->Initialize(context_.object3dCommon, context_.textureManager, sphereModel_);
	marker->GetTransform().translate = {x, -3.2f, z};
	marker->GetTransform().scale = {0.22f, 0.22f, 0.22f};
	marker->GetMaterialData()->color = {0.15f, 0.55f, 1.0f, 1.0f};
	return marker;
}

void GameScene::ResetGame() {
	// 動的な敵を破棄し、プレイヤーとゲーム進行用の値をまとめて初期化する
	enemyManager_.Reset();
	weaponManager_.Reset(enemyManager_);
	player_->Reset(parameters_.startingLives);
	parameterEditor_.SetPaused(false);
	score_ = 0;
	gameOver_ = false;
	for (size_t i = 0; i < railMarkers_.size(); ++i) {
		railMarkers_[i]->GetTransform().translate.z = static_cast<float>(i / 2) * 7.0f;
	}
}

void GameScene::UpdateRail() {
	// カメラ後方へ抜けたマーカーを前方へ循環させ、無限に続くレールとして見せる
	for (auto& marker : railMarkers_) {
		if (marker->GetTransform().translate.z < player_->GetCameraZ() - 2.0f) {
			marker->GetTransform().translate.z += 84.0f;
		}
		marker->Update(player_->GetCamera());
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
	UpdateRail();
	const float enemyDeltaTime = (!gameOver_ && !parameterEditor_.IsPaused()) ? deltaTime : 0.0f;
	enemyManager_.Update(player_->GetCamera(), enemyDeltaTime, parameters_.enemyRotationSpeed);
	score_ += weaponManager_.UpdateProjectiles(player_->GetCamera(), enemyDeltaTime, enemyManager_);

#ifdef _DEBUG
	// ゲームHUDとパラメータエディタは同じImGuiフレーム内へ構築する
	context_.imguiManager->BeginFrame();
	if (parameterEditor_.Draw(parameters_)) { ResetGame(); UpdateRail(); }
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
	player_->DrawShip();
	for (const auto& marker : railMarkers_) { marker->Draw(); }
	enemyManager_.Draw();
	weaponManager_.Draw();
	player_->DrawReticle();
}

void GameScene::Finalize() {
	// シーン所有物を解放してから、Frameworkへの非所有参照を破棄する
	initialized_ = false;
	weaponManager_.Reset(enemyManager_); enemyManager_.Reset(); railMarkers_.clear(); player_.reset();
	missileModel_.reset(); playerModel_.reset(); sphereModel_.reset();
	texture_ = 0; context_ = {};
}
