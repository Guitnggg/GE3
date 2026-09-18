#pragma once

#include "application/weapons/Bullet.h"
#include "application/weapons/Missile.h"
#include "engine/math/Mymath.h"

#include <cstdint>
#include <memory>
#include <vector>

class Camera;
class EnemyManager;
class Input;
class Model;
class Object3dCommon;
class TextureManager;

/// <summary>
/// 通常弾、ロックオン、追尾ミサイルの状態と更新を管理する。
/// </summary>
class WeaponManager final {
public:
	~WeaponManager();
	void Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager, Input* input,
		const std::shared_ptr<Model>& bulletModel, const std::shared_ptr<Model>& missileModel);
	void Reset(EnemyManager& enemies);
	void Shoot(const Vector3& origin, const Vector3& direction);
	void UpdateLockOn(float deltaTime, bool acceptMouseInput, const Vector3& origin,
		const Vector3& direction, const Vector3& missileOrigin, EnemyManager& enemies);
	uint32_t UpdateProjectiles(const Camera& camera, float deltaTime, EnemyManager& enemies);
	void OnEnemyRemoved(uint64_t id);
	void ClearLockOn(EnemyManager& enemies);
	void Draw() const;
	bool HasLock() const { return lockedEnemyId_ != 0; }

private:
	void LaunchMissile(const Vector3& origin, const Vector3& direction, const EnemyManager& enemies);
	uint32_t UpdateBullets(const Camera& camera, float deltaTime, EnemyManager& enemies);
	uint32_t UpdateMissiles(const Camera& camera, float deltaTime, EnemyManager& enemies);

	Object3dCommon* object3dCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	Input* input_ = nullptr;
	std::shared_ptr<Model> bulletModel_;
	std::shared_ptr<Model> missileModel_;
	std::vector<std::unique_ptr<Bullet>> bullets_;
	std::vector<std::unique_ptr<Missile>> missiles_;
	uint64_t lockedEnemyId_ = 0;
	float lockOnHoldTime_ = 0.0f;
};
