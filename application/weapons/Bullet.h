#pragma once

#include "engine/math/MathTypes.h"
#include "engine/collision/CollisionWorld.h"

#include <memory>

class Camera;
class Enemy;
class Model;
class Object3d;
class Object3dCommon;
class TextureManager;

/// <summary>通常攻撃の弾丸1発分の移動、寿命、描画、命中判定を管理する。</summary>
class Bullet final {
public:
	~Bullet();
	void Initialize(CollisionWorld* collisionWorld, Object3dCommon* object3dCommon, TextureManager* textureManager,
		const std::shared_ptr<Model>& model, const Vector3& position, const Vector3& direction);
	void Update(const Camera& camera, float deltaTime);
	void Draw() const;
	bool IsExpired() const { return remainingLifetime_ <= 0.0f; }
	uint64_t ConsumeHitEnemyId();

private:
	CollisionWorld* collisionWorld_ = nullptr;
	ColliderHandle collider_{};
	std::unique_ptr<Object3d> object_;
	Vector3 velocity_{};
	float remainingLifetime_ = 0.0f;
	float radius_ = 0.16f;
	uint64_t hitEnemyId_ = 0;
	static constexpr float kSpeed = 55.0f;
};
