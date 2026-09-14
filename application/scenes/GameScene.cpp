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
	if (initialized_) { throw std::logic_error("GameScene is already initialized."); }
	context_ = context;
	try {
		texture_ = context_.textureManager->Load("resource/monsterBall.png");
		sphereModel_ = context_.modelManager->Create(MeshGenerator::CreateSphere(kSphereSubdivisions), texture_);
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
	auto marker = std::make_unique<Object3d>();
	marker->Initialize(context_.object3dCommon, context_.textureManager, sphereModel_);
	marker->GetTransform().translate = {x, -3.2f, z};
	marker->GetTransform().scale = {0.22f, 0.22f, 0.22f};
	marker->GetMaterialData()->color = {0.15f, 0.55f, 1.0f, 1.0f};
	return marker;
}

void GameScene::ResetGame() {
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
	const Vector3 origin = player_->GetShotOrigin();
	const Vector3 direction = player_->GetShotDirection();
	float closest = std::numeric_limits<float>::max();
	size_t hitIndex = enemies_.size();
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
	for (auto& marker : railMarkers_) {
		if (marker->GetTransform().translate.z < player_->GetCameraZ() - 2.0f) {
			marker->GetTransform().translate.z += 84.0f;
		}
		marker->Update(player_->GetCamera());
	}
}

void GameScene::RemovePassedEnemies() {
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
	UpdateRail();
	const float enemyDeltaTime = (!gameOver_ && !parameterEditor_.IsPaused()) ? deltaTime : 0.0f;
	for (auto& enemy : enemies_) {
		enemy->Update(player_->GetCamera(), enemyDeltaTime, parameters_.enemyRotationSpeed);
	}

#ifdef _DEBUG
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

void GameScene::FixedUpdate() {}

void GameScene::Draw() {
	context_.object3dCommon->CommonDrawSetting();
	for (const auto& marker : railMarkers_) { marker->Draw(); }
	for (const auto& enemy : enemies_) { enemy->Draw(); }
	player_->DrawReticle();
}

void GameScene::Finalize() {
	initialized_ = false;
	enemies_.clear(); railMarkers_.clear(); player_.reset(); sphereModel_.reset();
	texture_ = 0; context_ = {};
}
