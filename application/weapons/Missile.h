#pragma once

#include "engine/math/MathTypes.h"
#include "engine/collision/CollisionWorld.h"

#include <cstdint>
#include <memory>

class Camera;
class Enemy;
class Model;
class Object3d;
class Object3dCommon;
class TextureManager;

/// <summary>
/// ロック対象をIDで追跡するミサイル1発分の誘導、寿命、描画を管理する。
/// </summary>
class Missile final {
  public:
	/// <summary>
	/// 衝突判定を解除してからミサイルを破棄する。
	/// </summary>
	~Missile();
	/// <summary>
	/// 発射情報と追尾対象IDからミサイルの描画物、速度、衝突判定を生成する。
	/// </summary>
	void Initialize(CollisionWorld *collisionWorld,
	                Object3dCommon *object3dCommon,
	                TextureManager *textureManager,
	                const std::shared_ptr<Model> &model,
	                const Vector3 &position,
	                const Vector3 &initialDirection,
	                uint64_t targetId);
	/// <summary>
	/// 有効な対象を追尾しながら移動し、寿命と衝突判定を更新する。
	/// </summary>
	void Update(const Camera &camera, float deltaTime, const Enemy *target);
	/// <summary>
	/// 現在のミサイルを描画する。
	/// </summary>
	void Draw() const;
	/// <summary>
	/// 寿命切れ、または命中済みで破棄可能か取得する。
	/// </summary>
	bool IsExpired() const {
		return remainingLifetime_ <= 0.0f;
	}
	/// <summary>
	/// 追尾対象として保持している敵IDを取得する。
	/// </summary>
	uint64_t GetTargetId() const {
		return targetId_;
	}
	/// <summary>
	/// 軌跡エフェクトなどに使用する現在位置を取得する。
	/// </summary>
	const Vector3 &GetPosition() const;
	/// <summary>
	/// 命中した敵IDを取得し、未命中状態へ戻す。
	/// </summary>
	uint64_t ConsumeHitEnemyId();

  private:
	/// <summary>
	/// 進行方向へ機体の見た目を回転させる。
	/// </summary>
	void UpdateRotation(float deltaTime, bool snap);

	CollisionWorld *collisionWorld_ = nullptr; // 衝突判定の登録先。所有しない
	ColliderHandle collider_{};
	std::unique_ptr<Object3d> object_;
	Vector3 velocity_{};
	uint64_t targetId_ = 0; // 追尾対象の安定した識別子。0は対象なし
	float remainingLifetime_ = 0.0f;
	uint64_t hitEnemyId_ = 0; // 未取得の命中結果。0は命中なし
	static constexpr float kRadius = 0.35f;
	static constexpr float kSpeed = 25.0f;
	static constexpr float kRotationSpeed = 8.0f;
};
