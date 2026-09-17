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
		texture_ = context_.textureManager->Load("resource/textures/monsterBall.png");
		sphereModel_ = context_.modelManager->Create(MeshGenerator::CreateSphere(kSphereSubdivisions), texture_);
		playerModel_ = context_.modelManager->Load("resource/models/player", "player.obj");
		missileModel_ = context_.modelManager->Load("resource/models/missile", "missile.obj");
		// プレイヤーを生成した後、進行方向の左右へレールマーカーを並べる
		player_ = std::make_unique<Player>();
		player_->Initialize(context_.spriteCommon, context_.object3dCommon,
			context_.textureManager, context_.input, playerModel_, texture_);
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
	bullets_.clear();
	missiles_.clear();
	player_->Reset(parameters_.startingLives);
	parameterEditor_.SetPaused(false);
	score_ = 0;
	spawnSequence_ = 0;
	nextEnemyId_ = 1;
	lockedEnemyId_ = 0;
	lockOnHoldTime_ = 0.0f;
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
		{xPositions[xIndex], yPositions[yIndex], player_->GetCameraZ() + parameters_.enemySpawnDistance},
		radius, nextEnemyId_++);
	enemies_.push_back(std::move(enemy));
	++spawnSequence_;
}

void GameScene::Shoot() {
	auto bullet = std::make_unique<Bullet>();
	bullet->Initialize(context_.object3dCommon, context_.textureManager, sphereModel_,
		player_->GetShotOrigin(), player_->GetShotDirection());
	bullets_.push_back(std::move(bullet));
}

void GameScene::UpdateBullets(float deltaTime) {
	for (auto bulletIt = bullets_.begin(); bulletIt != bullets_.end();) {
		(*bulletIt)->Update(player_->GetCamera(), deltaTime);
		auto hit = std::find_if(enemies_.begin(), enemies_.end(),
			[&bulletIt](const auto& enemy) { return (*bulletIt)->Intersects(*enemy); });
		if (hit != enemies_.end()) {
			if ((*hit)->GetId() == lockedEnemyId_) { lockedEnemyId_ = 0; }
			enemies_.erase(hit);
			++score_;
			bulletIt = bullets_.erase(bulletIt);
		} else if ((*bulletIt)->IsExpired()) {
			bulletIt = bullets_.erase(bulletIt);
		} else {
			++bulletIt;
		}
	}
}

Enemy* GameScene::FindEnemy(uint64_t id) const {
	if (id == 0) { return nullptr; }
	for (const auto& enemy : enemies_) {
		if (enemy->GetId() == id) { return enemy.get(); }
	}
	return nullptr;
}

void GameScene::ClearLockOn() {
	for (auto& enemy : enemies_) { enemy->SetLockedOn(false); }
	lockedEnemyId_ = 0;
}

void GameScene::UpdateLockOn(float deltaTime, bool acceptMouseInput) {
	if (!acceptMouseInput) { return; }
	const bool holding = context_.input->PushMouseButton(1);
	const bool released = context_.input->ReleaseMouseButton(1);
	if (released && lockedEnemyId_ != 0) { LaunchMissile(); }
	if (!holding) { lockOnHoldTime_ = 0.0f; ClearLockOn(); return; }
	lockOnHoldTime_ += deltaTime;
	// 短い右クリックを誤ロックにせず、ホールド操作として認識してから探索を始める
	constexpr float kLockOnDelay = 0.2f;
	if (lockOnHoldTime_ < kLockOnDelay) { ClearLockOn(); return; }

	// カーソルの射線に近い敵を、実際の当たり判定より広いロック範囲で選ぶ
	const Vector3 origin = player_->GetShotOrigin();
	const Vector3 direction = player_->GetShotDirection();
	Enemy* bestTarget = nullptr;
	float bestForward = std::numeric_limits<float>::max();
	for (const auto& enemy : enemies_) {
		const Vector3 offset{
			enemy->GetPosition().x - origin.x,
			enemy->GetPosition().y - origin.y,
			enemy->GetPosition().z - origin.z};
		const float forward = offset.x * direction.x + offset.y * direction.y + offset.z * direction.z;
		if (forward <= 0.0f) { continue; }
		const float distanceSquared = offset.x * offset.x + offset.y * offset.y + offset.z * offset.z;
		const float perpendicularSquared = std::max(0.0f, distanceSquared - forward * forward);
		const float lockRadius = enemy->GetRadius() + 2.5f;
		if (perpendicularSquared <= lockRadius * lockRadius && forward < bestForward) {
			bestForward = forward;
			bestTarget = enemy.get();
		}
	}

	for (auto& enemy : enemies_) { enemy->SetLockedOn(enemy.get() == bestTarget); }
	lockedEnemyId_ = bestTarget ? bestTarget->GetId() : 0;
}

void GameScene::LaunchMissile() {
	if (FindEnemy(lockedEnemyId_) == nullptr) { return; }
	auto missile = std::make_unique<Missile>();
	missile->Initialize(context_.object3dCommon, context_.textureManager, missileModel_,
		player_->GetPosition(), player_->GetShotDirection(), lockedEnemyId_);
	missiles_.push_back(std::move(missile));
}

void GameScene::UpdateMissiles(float deltaTime) {
	for (auto missileIt = missiles_.begin(); missileIt != missiles_.end();) {
		Enemy* target = FindEnemy((*missileIt)->GetTargetId());
		(*missileIt)->Update(player_->GetCamera(), deltaTime, target);
		if (target != nullptr && (*missileIt)->Intersects(*target)) {
			const uint64_t hitId = target->GetId();
			enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(),
				[hitId](const auto& enemy) { return enemy->GetId() == hitId; }), enemies_.end());
			if (lockedEnemyId_ == hitId) { lockedEnemyId_ = 0; }
			++score_;
			missileIt = missiles_.erase(missileIt);
		} else if ((*missileIt)->IsExpired()) {
			missileIt = missiles_.erase(missileIt);
		} else {
			++missileIt;
		}
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
		if ((*it)->IsPassed(player_->GetPosition().z)) {
			if ((*it)->GetId() == lockedEnemyId_) { lockedEnemyId_ = 0; }
			player_->Damage();
			it = enemies_.erase(it);
		} else { ++it; }
	}
	gameOver_ = player_->IsDead();
	if (gameOver_) { ClearLockOn(); }
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
			if (player_->Update(deltaTime, parameters_.railSpeed, parameters_.playerMoveSpeed, acceptFireInput)) { Shoot(); }
			UpdateLockOn(deltaTime, acceptFireInput);
			RemovePassedEnemies();
			if (!gameOver_) {
				enemySpawnTimer_ -= deltaTime;
				if (enemySpawnTimer_ <= 0.0f) {
					SpawnEnemy();
					enemySpawnTimer_ = std::max(parameters_.minimumSpawnInterval,
						parameters_.enemySpawnInterval - static_cast<float>(score_) * parameters_.spawnAccelerationPerScore);
				}
			}
		}
	} else if (context_.input->TriggerKey(DIK_R)) { ResetGame(); }
	// 停止中も描画行列は更新し、現在の画面をそのまま表示できるようにする
	UpdateRail();
	const float enemyDeltaTime = (!gameOver_ && !parameterEditor_.IsPaused()) ? deltaTime : 0.0f;
	for (auto& enemy : enemies_) {
		enemy->Update(player_->GetCamera(), enemyDeltaTime, parameters_.enemyRotationSpeed);
	}
	UpdateBullets(enemyDeltaTime);
	UpdateMissiles(enemyDeltaTime);

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
	if (lockedEnemyId_ != 0) { ImGui::TextColored({1.0f, 0.85f, 0.1f, 1.0f}, "LOCKED"); }
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
	for (const auto& enemy : enemies_) { enemy->Draw(); }
	for (const auto& bullet : bullets_) { bullet->Draw(); }
	for (const auto& missile : missiles_) { missile->Draw(); }
	player_->DrawReticle();
}

void GameScene::Finalize() {
	// シーン所有物を解放してから、Frameworkへの非所有参照を破棄する
	initialized_ = false;
	enemies_.clear(); bullets_.clear(); missiles_.clear(); railMarkers_.clear(); player_.reset();
	missileModel_.reset(); playerModel_.reset(); sphereModel_.reset();
	texture_ = 0; context_ = {};
}
