#include "application/managers/EnemyManager.h"

#include "application/characters/Enemy.h"
#include "application/editor/GameParameterEditor.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

EnemyManager::~EnemyManager() = default;

void EnemyManager::Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
	const std::shared_ptr<Model>& model) {
	if (!object3dCommon || !textureManager || !model) {
		throw std::invalid_argument("EnemyManager requires initialized rendering services and a model.");
	}
	object3dCommon_ = object3dCommon;
	textureManager_ = textureManager;
	model_ = model;
	Reset();
}

void EnemyManager::Reset() {
	enemies_.clear();
	spawnSequence_ = 0;
	nextEnemyId_ = 1;
	spawnTimer_ = 0.2f;
}

void EnemyManager::Spawn(float cameraZ, const GameParameters& parameters) {
	constexpr float xPositions[] = {-4.2f, -2.1f, 0.0f, 2.1f, 4.2f};
	constexpr float yPositions[] = {-1.6f, 0.0f, 1.7f, -0.7f};
	const uint32_t xIndex = (spawnSequence_ * 3u + spawnSequence_ / 2u) % 5u;
	const uint32_t yIndex = (spawnSequence_ * 5u + 1u) % 4u;
	const float radius = parameters.enemyBaseRadius +
		static_cast<float>(spawnSequence_ % 3u) * parameters.enemyRadiusStep;
	auto enemy = std::make_unique<Enemy>();
	enemy->Initialize(object3dCommon_, textureManager_, model_,
		{xPositions[xIndex], yPositions[yIndex], cameraZ + parameters.enemySpawnDistance},
		radius, nextEnemyId_++);
	enemies_.push_back(std::move(enemy));
	++spawnSequence_;
}

void EnemyManager::UpdateSpawning(float deltaTime, float cameraZ,
	const GameParameters& parameters, uint32_t score) {
	spawnTimer_ -= deltaTime;
	if (spawnTimer_ > 0.0f) { return; }
	Spawn(cameraZ, parameters);
	spawnTimer_ = std::max(parameters.minimumSpawnInterval,
		parameters.enemySpawnInterval - static_cast<float>(score) * parameters.spawnAccelerationPerScore);
}

void EnemyManager::Update(const Camera& camera, float deltaTime, float rotationSpeed) {
	for (auto& enemy : enemies_) { enemy->Update(camera, deltaTime, rotationSpeed); }
}

void EnemyManager::Draw() const {
	for (const auto& enemy : enemies_) { enemy->Draw(); }
}

Enemy* EnemyManager::Find(uint64_t id) const {
	if (id == 0) { return nullptr; }
	for (const auto& enemy : enemies_) {
		if (enemy->GetId() == id) { return enemy.get(); }
	}
	return nullptr;
}

Enemy* EnemyManager::FindLockTarget(const Vector3& origin, const Vector3& direction) const {
	Enemy* bestTarget = nullptr;
	float bestForward = std::numeric_limits<float>::max();
	for (const auto& enemy : enemies_) {
		const Vector3 offset{enemy->GetPosition().x - origin.x, enemy->GetPosition().y - origin.y,
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
	return bestTarget;
}

void EnemyManager::SetLockedEnemy(uint64_t id) {
	for (auto& enemy : enemies_) { enemy->SetLockedOn(enemy->GetId() == id); }
}

bool EnemyManager::Remove(uint64_t id) {
	const auto oldSize = enemies_.size();
	enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(),
		[id](const auto& enemy) { return enemy->GetId() == id; }), enemies_.end());
	return enemies_.size() != oldSize;
}

std::vector<uint64_t> EnemyManager::RemovePassed(float playerZ) {
	std::vector<uint64_t> removedIds;
	for (auto it = enemies_.begin(); it != enemies_.end();) {
		if ((*it)->IsPassed(playerZ)) {
			removedIds.push_back((*it)->GetId());
			it = enemies_.erase(it);
		} else {
			++it;
		}
	}
	return removedIds;
}
