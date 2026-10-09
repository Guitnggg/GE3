#pragma once

#include "application/weapons/Bullet.h"
#include "application/weapons/Missile.h"
#include "engine/math/MathTypes.h"
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
	/// <summary>
	/// 
	/// </summary>
	~WeaponManager();

	/// <summary>
	/// 
	/// </summary>
	/// <param name="collisionWorld"></param>
	/// <param name="object3dCommon"></param>
	/// <param name="textureManager"></param>
	/// <param name="input"></param>
	/// <param name="bulletModel"></param>
	/// <param name="missileModel"></param>
	/// <param name="lockOnAction"></param>
	void Initialize(CollisionWorld *collisionWorld,
	                Object3dCommon *object3dCommon,
	                TextureManager *textureManager,
	                Input *input,
	                const std::shared_ptr<Model> &bulletModel,
	                const std::shared_ptr<Model> &missileModel,
	                InputActionId lockOnAction);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="enemies"></param>
	void Reset(EnemyManager &enemies);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="origin"></param>
	/// <param name="direction"></param>
	void Shoot(const Vector3 &origin, const Vector3 &direction);

	/// <summary>
	/// ロックオンの状態を更新する。経過時間や入力、射出方向・位置、および敵情報に基づいて目標の選択や追従を行う。
	/// </summary>
	/// <param name="deltaTime">前フレームからの経過時間（秒）。更新処理の時間ステップとして使用される。</param>
	/// <param name="acceptMouseInput">マウス入力によるターゲット選択を許可するかどうかを示すフラグ。</param>
	/// <param name="direction">発射または視線の方向を示すVector3（通常は単位ベクトル）。ロックオンの向きを決定するために使用される。</param>
	/// <param name="missileOrigin">ミサイルや発射体の起点位置を示すVector3。判定の基準点として使用される。</param>
	/// <param name="enemies">EnemyManagerへの参照。現在の敵一覧や状態を取得・更新してロックオン対象を決定するために使用される。</param>
	void UpdateLockOn(float deltaTime,
	                  bool acceptMouseInput,
	                  const Vector3 &direction,
	                  const Vector3 &missileOrigin,
	                  EnemyManager &enemies);
	void UpdateProjectiles(const Camera &camera, float deltaTime, EnemyManager &enemies);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="enemies"></param>
	/// <returns></returns>
	std::vector<Vector3> ResolveProjectileHits(EnemyManager &enemies);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="id"></param>
	void OnEnemyRemoved(uint64_t id);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="enemies"></param>
	void ClearLockOn(EnemyManager &enemies);

	/// <summary>
	/// 
	/// </summary>
	void Draw() const;

	std::vector<Vector3> GetMissilePositions() const;

	bool HasLock() const {
		return !lockedEnemyIds_.empty();
	}
	size_t GetLockCount() const {
		return lockedEnemyIds_.size();
	}

  private:
	/// <summary>
	/// 
	/// </summary>
	/// <param name="origin"></param>
	/// <param name="direction"></param>
	/// <param name="enemies"></param>
	void LaunchMissiles(const Vector3 &origin, const Vector3 &direction, const EnemyManager &enemies);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="camera"></param>
	/// <param name="deltaTime"></param>
	void UpdateBullets(const Camera &camera, float deltaTime);

	/// <summary>
	/// カメラ情報と経過時間、および敵管理を利用してミサイルの位置・状態・当たり判定を更新する。
	/// </summary>
	/// <param name="camera">ビューや表示領域などのカメラ情報へのconst参照。ミサイルの可視判定や座標変換に使用される。</param>
	/// <param name="deltaTime">前フレームからの経過時間（秒）。ミサイルの移動やアニメーションの時間更新に使用される。</param>
	/// <param name="enemies">EnemyManagerへの参照。命中判定や敵への影響（ダメージ適用や状態変更）などで更新される可能性がある。</param>
	void UpdateMissiles(const Camera &camera, float deltaTime, EnemyManager &enemies);

	CollisionWorld *collisionWorld_ = nullptr;
	Object3dCommon *object3dCommon_ = nullptr;
	TextureManager *textureManager_ = nullptr;
	Input *input_ = nullptr;
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
