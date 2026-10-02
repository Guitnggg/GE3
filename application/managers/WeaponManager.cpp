#include "application/managers/WeaponManager.h"

#include "application/characters/Enemy.h"
#include "application/managers/EnemyManager.h"
#include "application/weapons/Bullet.h"
#include "application/weapons/Missile.h"
#include "engine/input/Input.h"
#include "engine/collision/CollisionWorld.h"

#include <algorithm>
#include <stdexcept>

WeaponManager::~WeaponManager() = default;

void WeaponManager::Initialize(CollisionWorld* collisionWorld, Object3dCommon* object3dCommon, TextureManager* textureManager, Input* input,
	const std::shared_ptr<Model>& bulletModel, const std::shared_ptr<Model>& missileModel,
	InputActionId lockOnAction) {
	if (!collisionWorld || !object3dCommon || !textureManager || !input || !bulletModel || !missileModel) {
		throw std::invalid_argument("WeaponManager requires initialized services and models.");
	}
	collisionWorld_ = collisionWorld;
	object3dCommon_ = object3dCommon;
	textureManager_ = textureManager;
	input_ = input;
	lockOnAction_ = lockOnAction;
	bulletModel_ = bulletModel;
	missileModel_ = missileModel;
}

void WeaponManager::Reset(EnemyManager& enemies) {
	bullets_.clear();
	missiles_.clear();
	lockOnHoldTime_ = 0.0f;
	ClearLockOn(enemies);
}

void WeaponManager::Shoot(const Vector3& origin, const Vector3& direction) {
	auto bullet = std::make_unique<Bullet>();
	bullet->Initialize(collisionWorld_, object3dCommon_, textureManager_, bulletModel_, origin, direction);
	bullets_.push_back(std::move(bullet));
}

void WeaponManager::UpdateLockOn(float deltaTime, bool acceptMouseInput, const Vector3& direction,
	const Vector3& missileOrigin, EnemyManager& enemies) {
	if (!acceptMouseInput) { return; }
	const bool holding = input_->PushAction(lockOnAction_);
	const bool released = input_->ReleaseAction(lockOnAction_);
	if (released) {
		LaunchMissiles(missileOrigin, direction, enemies);
		ClearLockOn(enemies);
		return;
	}
	if (!holding) { ClearLockOn(enemies); return; }
	if (lockedEnemyIds_.size() >= kMaxLockCount) { return; }
	lockOnHoldTime_ += deltaTime;
	while (lockOnHoldTime_ >= kLockInterval && lockedEnemyIds_.size() < kMaxLockCount) {
		lockOnHoldTime_ -= kLockInterval;
		Enemy* target = enemies.FindNearestLockTarget(missileOrigin, lockedEnemyIds_);
		if (target == nullptr) { break; }
		lockedEnemyIds_.push_back(target->GetId());
		enemies.SetLockedEnemies(lockedEnemyIds_);
	}
}

void WeaponManager::LaunchMissiles(const Vector3& origin, const Vector3& direction,
	const EnemyManager& enemies) {
	for (const uint64_t targetId : lockedEnemyIds_) {
		if (enemies.Find(targetId) == nullptr) { continue; }
		auto missile = std::make_unique<Missile>();
		missile->Initialize(collisionWorld_, object3dCommon_, textureManager_, missileModel_, origin, direction, targetId);
		missiles_.push_back(std::move(missile));
	}
}

void WeaponManager::UpdateBullets(const Camera& camera, float deltaTime) {
	for (auto bulletIt = bullets_.begin(); bulletIt != bullets_.end();) {
		(*bulletIt)->Update(camera, deltaTime);
		if ((*bulletIt)->IsExpired()) {
			bulletIt = bullets_.erase(bulletIt);
		} else {
			++bulletIt;
		}
	}
}

void WeaponManager::UpdateMissiles(const Camera& camera, float deltaTime, EnemyManager& enemies) {
	for (auto missileIt = missiles_.begin(); missileIt != missiles_.end();) {
		Enemy* target = enemies.Find((*missileIt)->GetTargetId());
		(*missileIt)->Update(camera, deltaTime, target);
		if ((*missileIt)->IsExpired()) {
			missileIt = missiles_.erase(missileIt);
		} else {
			++missileIt;
		}
	}
}

void WeaponManager::UpdateProjectiles(const Camera& camera, float deltaTime, EnemyManager& enemies) {
	UpdateBullets(camera, deltaTime);
	UpdateMissiles(camera, deltaTime, enemies);
}

uint32_t WeaponManager::ResolveProjectileHits(EnemyManager& enemies) {
	uint32_t killCount = 0;
	for (auto bullet = bullets_.begin(); bullet != bullets_.end();) {
		const uint64_t hitId = (*bullet)->ConsumeHitEnemyId();
		if (hitId == 0) { ++bullet; continue; }
		if (enemies.Remove(hitId)) { OnEnemyRemoved(hitId); ++killCount; }
		bullet = bullets_.erase(bullet);
	}
	for (auto missile = missiles_.begin(); missile != missiles_.end();) {
		const uint64_t hitId = (*missile)->ConsumeHitEnemyId();
		if (hitId == 0) { ++missile; continue; }
		if (enemies.Remove(hitId)) { OnEnemyRemoved(hitId); ++killCount; }
		missile = missiles_.erase(missile);
	}
	return killCount;
}

void WeaponManager::OnEnemyRemoved(uint64_t id) {
	lockedEnemyIds_.erase(std::remove(lockedEnemyIds_.begin(), lockedEnemyIds_.end(), id), lockedEnemyIds_.end());
}

void WeaponManager::ClearLockOn(EnemyManager& enemies) {
	lockedEnemyIds_.clear();
	lockOnHoldTime_ = 0.0f;
	enemies.SetLockedEnemies(lockedEnemyIds_);
}

void WeaponManager::Draw() const {
	for (const auto& bullet : bullets_) { bullet->Draw(); }
	for (const auto& missile : missiles_) { missile->Draw(); }
}
