#include "application/scenes/GameScene.h"

#include "application/characters/Enemy.h"
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
#include <algorithm>
#include <limits>
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
		texture_ = context_.textureManager->Load("resource/monsterBall.png");
		sphereModel_ = context_.modelManager->Create(MeshGenerator::CreateSphere(kSphereSubdivisions), texture_);
		// プレイヤーを生成した後、進行方向の左右へレールマーカーを並べる
		player_ = std::make_unique<Player>();
		player_->Initialize(context_.spriteCommon, context_.textureManager, context_.input, texture_);
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
	enemies_.clear();
	player_->Reset(parameters_.startingLives);
	parameterEditor_.SetPaused(false);
	score_ = 0;
	spawnSequence_ = 0;
	enemySpawnTimer_ = 0.2f;
	gameOver_ = false;
	for (size_t i = 0; i < railMarkers_.size(); ++i) {
		railMarkers_[i]->GetTransform().translate.z = static_cast<float>(i / 2) * 7.0f;
	}
}

void GameScene::SpawnEnemy() {
	// 再現可能な列パターンから位置を選び、エディタの値で距離と大きさを決める
	constexpr float xPositions[] = {-4.2f, -2.1f, 0.0f, 2.1f, 4.2f};
	constexpr float yPositions[] = {-1.6f, 0.0f, 1.7f, -0.7f};
	const uint32_t xIndex = (spawnSequence_ * 3u + spawnSequence_ / 2u) % 5u;
	const uint32_t yIndex = (spawnSequence_ * 5u + 1u) % 4u;
	const float radius = parameters_.enemyBaseRadius +
		static_cast<float>(spawnSequence_ % 3u) * parameters_.enemyRadiusStep;
	auto enemy = std::make_unique<Enemy>();
	enemy->Initialize(context_.object3dCommon, context_.textureManager, sphereModel_,
		{xPositions[xIndex], yPositions[yIndex], player_->GetCameraZ() + parameters_.enemySpawnDistance}, radius);
	enemies_.push_back(std::move(enemy));
	++spawnSequence_;
}

void GameScene::Shoot() {
	// 画面上の照準からワールド空間の射線を受け取る
	const Vector3 origin = player_->GetShotOrigin();
	const Vector3 direction = player_->GetShotDirection();
	float closest = std::numeric_limits<float>::max();
	size_t hitIndex = enemies_.size();
	// 射線上に複数の敵がいる場合はカメラに最も近い1体だけを撃破する
	for (size_t i = 0; i < enemies_.size(); ++i) {
		float distance = 0.0f;
		if (enemies_[i]->IntersectsRay(origin, direction, distance) && distance < closest) {
			closest = distance;
			hitIndex = i;
		}
	}
	if (hitIndex != enemies_.size()) {
		enemies_.erase(enemies_.begin() + hitIndex);
		++score_;
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

void GameScene::RemovePassedEnemies() {
	// 撃破されずカメラまで到達した敵を消し、1体につきライフを1減らす
	for (auto it = enemies_.begin(); it != enemies_.end();) {
		if ((*it)->IsPassed(player_->GetCameraZ())) {
			player_->Damage();
			it = enemies_.erase(it);
		} else { ++it; }
	}
	gameOver_ = player_->IsDead();
}

void GameScene::Update() {
	const float deltaTime = context_.time->GetDeltaTime();
	// 一時停止中とゲームオーバー中はゲームロジックを進めない
	if (!gameOver_) {
		if (!parameterEditor_.IsPaused()) {
			if (player_->Update(deltaTime, parameters_.railSpeed, parameters_.aimSpeed)) { Shoot(); }
			RemovePassedEnemies();
			enemySpawnTimer_ -= deltaTime;
			if (enemySpawnTimer_ <= 0.0f) {
				SpawnEnemy();
				enemySpawnTimer_ = std::max(parameters_.minimumSpawnInterval,
					parameters_.enemySpawnInterval - static_cast<float>(score_) * parameters_.spawnAccelerationPerScore);
			}
		}
	} else if (context_.input->TriggerKey(DIK_R)) { ResetGame(); }
	// 停止中も描画行列は更新し、現在の画面をそのまま表示できるようにする
	UpdateRail();
	const float enemyDeltaTime = (!gameOver_ && !parameterEditor_.IsPaused()) ? deltaTime : 0.0f;
	for (auto& enemy : enemies_) {
		enemy->Update(player_->GetCamera(), enemyDeltaTime, parameters_.enemyRotationSpeed);
	}

#ifdef _DEBUG
	// ゲームHUDとパラメータエディタは同じImGuiフレーム内へ構築する
	context_.imguiManager->BeginFrame();
	if (parameterEditor_.Draw(parameters_)) { ResetGame(); UpdateRail(); }
	ImGui::SetNextWindowPos({12.0f, 12.0f}, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.72f);
	ImGui::Begin("3D RAIL SHOOTER", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::Text("SCORE  %u", score_);
	ImGui::Text("LIVES  %u", player_->GetLives());
	ImGui::TextUnformatted("AIM: WASD / Arrow Keys    FIRE: Space");
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
	for (const auto& marker : railMarkers_) { marker->Draw(); }
	for (const auto& enemy : enemies_) { enemy->Draw(); }
	player_->DrawReticle();
}

void GameScene::Finalize() {
	// シーン所有物を解放してから、Frameworkへの非所有参照を破棄する
	initialized_ = false;
	enemies_.clear(); railMarkers_.clear(); player_.reset(); sphereModel_.reset();
	texture_ = 0; context_ = {};
}
