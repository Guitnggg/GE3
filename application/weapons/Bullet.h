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

/// <summary>
/// 通常攻撃の弾丸1発分の移動、寿命、描画、命中判定を管理する。
/// </summary>
class Bullet final {
  public:
	/// <summary>
	/// 衝突判定を解除してから弾丸を破棄する。
	/// </summary>
	~Bullet();
	/// <summary>
	/// 発射位置と方向から弾丸の描画物、速度、衝突判定を生成する。
	/// </summary>
	void Initialize(CollisionWorld *collisionWorld,
	                Object3dCommon *object3dCommon,
	                TextureManager *textureManager,
	                const std::shared_ptr<Model> &model,
	                const Vector3 &position,
	                const Vector3 &direction);
	/// <summary>
	/// 弾丸を移動し、寿命と衝突判定を更新する。
	/// </summary>
	void Update(const Camera &camera, float deltaTime);
	/// <summary>
	/// 現在の弾丸を描画する。
	/// </summary>
	void Draw() const;
	/// <summary>
	/// 寿命切れ、または命中済みで破棄可能か取得する。
	/// </summary>
	bool IsExpired() const {
		return remainingLifetime_ <= 0.0f;
	}
	/// <summary>
	/// 命中した敵IDを取得し、未命中状態へ戻す。
	/// </summary>
	uint64_t ConsumeHitEnemyId();

  private:
	CollisionWorld *collisionWorld_ = nullptr; // 衝突判定の登録先。所有しない
	ColliderHandle collider_{};
	std::unique_ptr<Object3d> object_;
	Vector3 velocity_{};
	float remainingLifetime_ = 0.0f;
	float radius_ = 0.16f;
	uint64_t hitEnemyId_ = 0; // 未取得の命中結果。0は命中なし
	static constexpr float kSpeed = 55.0f;
};
