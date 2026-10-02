#pragma once

#include "application/characters/Enemy.h"
#include "engine/math/Mymath.h"

#include <cstdint>
#include <memory>
#include <vector>

class Camera;
class CollisionWorld;
struct GameParameters;
class Model;
class Object3dCommon;
class SpriteCommon;
class TextureManager;

/// <summary>
/// 敵の生成、所有、更新、検索、削除をまとめて管理する。
/// </summary>
class EnemyManager final {
public:
	~EnemyManager();
	void Initialize(CollisionWorld* collisionWorld, SpriteCommon* spriteCommon,
		Object3dCommon* object3dCommon, TextureManager* textureManager,
		const std::shared_ptr<Model>& model, uint32_t lockOnTexture);
	void Reset();
	void Update(const Camera& camera, float deltaTime, float rotationSpeed);
	void UpdateSpawning(float deltaTime, float cameraZ, const GameParameters& parameters, uint32_t score);
	void Draw() const;
	void DrawLockOnMarkers() const;

	Enemy* Find(uint64_t id) const;
	Enemy* FindNearestLockTarget(const Vector3& origin, const std::vector<uint64_t>& excludedIds) const;
	void SetLockedEnemies(const std::vector<uint64_t>& ids);
	bool Remove(uint64_t id);
	std::vector<uint64_t> RemovePassed(float playerZ);
private:
	void Spawn(float cameraZ, const GameParameters& parameters);

	Object3dCommon* object3dCommon_ = nullptr;
	SpriteCommon* spriteCommon_ = nullptr;
	CollisionWorld* collisionWorld_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	std::shared_ptr<Model> model_;
	uint32_t lockOnTexture_ = 0;
	std::vector<std::unique_ptr<Enemy>> enemies_;
	uint32_t spawnSequence_ = 0;
	uint64_t nextEnemyId_ = 1;
	float spawnTimer_ = 0.0f;
};
