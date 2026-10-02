#pragma once

#include "application/weapons/Bullet.h"
#include "application/weapons/Missile.h"
#include "engine/math/Mymath.h"
#include "engine/input/Input.h"

#include <cstdint>
#include <memory>
#include <vector>

class Camera;
class CollisionWorld;
class EnemyManager;
class Model;
class Object3dCommon;
class TextureManager;

/// <summary>
/// 通常弾、ロックオン、追尾ミサイルの状態と更新を管理する。
/// </summary>
class WeaponManager final {
public:
	~WeaponManager();
	void Initialize(CollisionWorld* collisionWorld, Object3dCommon* object3dCommon, TextureManager* textureManager, Input* input,
		const std::shared_ptr<Model>& bulletModel, const std::shared_ptr<Model>& missileModel,
		InputActionId lockOnAction);
	void Reset(EnemyManager& enemies);
	void Shoot(const Vector3& origin, const Vector3& direction);
	void UpdateLockOn(float deltaTime, bool acceptMouseInput, const Vector3& direction,
		const Vector3& missileOrigin, EnemyManager& enemies);
	void UpdateProjectiles(const Camera& camera, float deltaTime, EnemyManager& enemies);
	/// <summary>命中を解決し、このフレームに撃破した敵のワールド座標を返す。</summary>
	std::vector<Vector3> ResolveProjectileHits(EnemyManager& enemies);
	void OnEnemyRemoved(uint64_t id);
	void ClearLockOn(EnemyManager& enemies);
	void Draw() const;
	std::vector<Vector3> GetMissilePositions() const;
	bool HasLock() const { return !lockedEnemyIds_.empty(); }
	size_t GetLockCount() const { return lockedEnemyIds_.size(); }

private:
	void LaunchMissiles(const Vector3& origin, const Vector3& direction, const EnemyManager& enemies);
	void UpdateBullets(const Camera& camera, float deltaTime);
	void UpdateMissiles(const Camera& camera, float deltaTime, EnemyManager& enemies);

	CollisionWorld* collisionWorld_ = nullptr;
	Object3dCommon* object3dCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	Input* input_ = nullptr;
	InputActionId lockOnAction_ = kInvalidInputActionId;
	std::shared_ptr<Model> bulletModel_;
	std::shared_ptr<Model> missileModel_;
	std::vector<std::unique_ptr<Bullet>> bullets_;
	std::vector<std::unique_ptr<Missile>> missiles_;
	std::vector<uint64_t> lockedEnemyIds_;
	float lockOnHoldTime_ = 0.0f;
	static constexpr size_t kMaxLockCount = 5;
	static constexpr float kLockInterval = 0.2f;
};
