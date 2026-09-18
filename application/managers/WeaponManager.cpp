#include "application/managers/WeaponManager.h"

#include "application/characters/Enemy.h"
#include "application/managers/EnemyManager.h"
#include "application/weapons/Bullet.h"
#include "application/weapons/Missile.h"
#include "engine/input/Input.h"

#include <algorithm>
#include <stdexcept>

WeaponManager::~WeaponManager() = default;

void WeaponManager::Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager, Input* input,
	const std::shared_ptr<Model>& bulletModel, const std::shared_ptr<Model>& missileModel) {
	if (!object3dCommon || !textureManager || !input || !bulletModel || !missileModel) {
		throw std::invalid_argument("WeaponManager requires initialized services and models.");
	}
	object3dCommon_ = object3dCommon;
	textureManager_ = textureManager;
	input_ = input;
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
	bullet->Initialize(object3dCommon_, textureManager_, bulletModel_, origin, direction);
	bullets_.push_back(std::move(bullet));
}

void WeaponManager::UpdateLockOn(float deltaTime, bool acceptMouseInput, const Vector3& origin,
	const Vector3& direction, const Vector3& missileOrigin, EnemyManager& enemies) {
	if (!acceptMouseInput) { return; }
	const bool holding = input_->PushMouseButton(1);
	const bool released = input_->ReleaseMouseButton(1);
	if (released && lockedEnemyId_ != 0) { LaunchMissile(missileOrigin, direction, enemies); }
	if (!holding) { lockOnHoldTime_ = 0.0f; ClearLockOn(enemies); return; }
	lockOnHoldTime_ += deltaTime;
	constexpr float kLockOnDelay = 0.2f;
	if (lockOnHoldTime_ < kLockOnDelay) { ClearLockOn(enemies); return; }
	Enemy* target = enemies.FindLockTarget(origin, direction);
	lockedEnemyId_ = target ? target->GetId() : 0;
	enemies.SetLockedEnemy(lockedEnemyId_);
}

void WeaponManager::LaunchMissile(const Vector3& origin, const Vector3& direction,
	const EnemyManager& enemies) {
	if (enemies.Find(lockedEnemyId_) == nullptr) { return; }
	auto missile = std::make_unique<Missile>();
	missile->Initialize(object3dCommon_, textureManager_, missileModel_, origin, direction, lockedEnemyId_);
	missiles_.push_back(std::move(missile));
}

uint32_t WeaponManager::UpdateBullets(const Camera& camera, float deltaTime, EnemyManager& enemies) {
	uint32_t killCount = 0;
	for (auto bulletIt = bullets_.begin(); bulletIt != bullets_.end();) {
		(*bulletIt)->Update(camera, deltaTime);
		const auto& enemyList = enemies.GetEnemies();
		const auto hit = std::find_if(enemyList.begin(), enemyList.end(),
			[&bulletIt](const auto& enemy) { return (*bulletIt)->Intersects(*enemy); });
		if (hit != enemyList.end()) {
			const uint64_t hitId = (*hit)->GetId();
			enemies.Remove(hitId);
			OnEnemyRemoved(hitId);
			++killCount;
			bulletIt = bullets_.erase(bulletIt);
		} else if ((*bulletIt)->IsExpired()) {
			bulletIt = bullets_.erase(bulletIt);
		} else {
			++bulletIt;
		}
	}
	return killCount;
}

uint32_t WeaponManager::UpdateMissiles(const Camera& camera, float deltaTime, EnemyManager& enemies) {
	uint32_t killCount = 0;
	for (auto missileIt = missiles_.begin(); missileIt != missiles_.end();) {
		Enemy* target = enemies.Find((*missileIt)->GetTargetId());
		(*missileIt)->Update(camera, deltaTime, target);
		if (target != nullptr && (*missileIt)->Intersects(*target)) {
			const uint64_t hitId = target->GetId();
			enemies.Remove(hitId);
			OnEnemyRemoved(hitId);
			++killCount;
			missileIt = missiles_.erase(missileIt);
		} else if ((*missileIt)->IsExpired()) {
			missileIt = missiles_.erase(missileIt);
		} else {
			++missileIt;
		}
	}
	return killCount;
}

uint32_t WeaponManager::UpdateProjectiles(const Camera& camera, float deltaTime, EnemyManager& enemies) {
	return UpdateBullets(camera, deltaTime, enemies) + UpdateMissiles(camera, deltaTime, enemies);
}

void WeaponManager::OnEnemyRemoved(uint64_t id) {
	if (lockedEnemyId_ == id) { lockedEnemyId_ = 0; }
}

void WeaponManager::ClearLockOn(EnemyManager& enemies) {
	lockedEnemyId_ = 0;
	enemies.SetLockedEnemy(0);
}

void WeaponManager::Draw() const {
	for (const auto& bullet : bullets_) { bullet->Draw(); }
	for (const auto& missile : missiles_) { missile->Draw(); }
}
