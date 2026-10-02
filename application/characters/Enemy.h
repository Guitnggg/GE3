#pragma once

#include "engine/3D/object/Object3d.h"
#include "engine/collision/CollisionWorld.h"
#include <cstdint>
#include <memory>

class Camera;
class Model;
class Object3dCommon;
class TextureManager;

/// <summary>
/// 1体の敵の表示、通過判定、射線判定を管理する。
/// </summary>
class Enemy final {
public:
	~Enemy();
	/// <summary>
	/// 共有球モデルを使用して敵の3Dオブジェクトを生成する。
	/// </summary>
	/// <param name="object3dCommon">3Dオブジェクトの共通描画機能</param>
	/// <param name="textureManager">テクスチャ管理機能</param>
	/// <param name="model">全敵で共有する球モデル</param>
	/// <param name="position">敵を配置するワールド座標</param>
	/// <param name="radius">表示スケールと当たり判定に使用する半径</param>
	void Initialize(CollisionWorld* collisionWorld, Object3dCommon* object3dCommon, TextureManager* textureManager,
		const std::shared_ptr<Model>& model, const Vector3& position, float radius, uint64_t id);

	/// <summary>
	/// 回転を進め、現在のカメラに対する描画行列を更新する。
	/// </summary>
	void Update(const Camera& camera, float deltaTime, float rotationSpeed);

	/// <summary>
	/// 敵の3Dモデルを描画する。
	/// </summary>
	void Draw() const;

	/// <summary>
	/// 敵がカメラ位置を通過したか判定する。
	/// </summary>
	bool IsPassed(float cameraZ) const;

	/// <summary>ロックオン表示の有無を色へ反映する。</summary>
	void SetLockedOn(bool lockedOn);
	const Vector3& GetPosition() const { return object_->GetTransform().translate; }
	float GetRadius() const { return radius_; }
	uint64_t GetId() const { return id_; }

private:
	CollisionWorld* collisionWorld_ = nullptr;
	ColliderHandle collider_{};
	std::unique_ptr<Object3d> object_; // 敵の表示とワールド座標を所有する3Dオブジェクト
	float radius_ = 1.0f;             // 射線判定に使用する球の半径
	uint64_t id_ = 0;                 // ミサイルが安全に追跡するための一意な番号
};
