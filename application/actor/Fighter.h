#pragma once

#include "application/combat/Damage.h"
#include "application/combat/Team.h"
#include "engine/3D/object/Object3d.h"

#include <memory>

class Camera;
class Model;
class Object3dCommon;
class TextureManager;
class PlayerController;
class EnemyController;
class GameEditor;

/// <summary>
/// プレイヤー機と敵機で共有する、機体の状態と表示処理を管理するクラス
/// </summary>
class Fighter final {
  public:
	/// <summary>
	/// 機体モデル、所属チーム、表示色を初期化する
	/// </summary>
	void Initialize(Object3dCommon *object3dCommon, TextureManager *textureManager,
	                const std::shared_ptr<Model> &model, Team team,
	                const Vector4 &color = {1.0f, 1.0f, 1.0f, 1.0f});

	/// <summary>
	/// 戦闘開始時の位置、向き、速度、耐久値へ戻す
	/// </summary>
	void Reset(const Vector3 &position, float yaw, float speed);

	/// <summary>
	/// 現在の機体状態を3Dモデルへ反映する
	/// </summary>
	void UpdateVisual(const Camera &camera);

	/// <summary>
	/// 生存している機体を描画する
	/// </summary>
	void Draw() const;

	/// <summary>
	/// 指定されたダメージを耐久値へ反映する
	/// </summary>
	void ApplyDamage(const Damage &damage);

	// ===== 機体状態の参照 =====

	[[nodiscard]] const Vector3 &GetPosition() const { return position_; }
	[[nodiscard]] float GetYaw() const { return yaw_; }
	[[nodiscard]] float GetPitch() const { return pitch_; }
	[[nodiscard]] float GetRoll() const { return roll_; }
	[[nodiscard]] float GetSpeed() const { return speed_; }
	[[nodiscard]] int GetHealth() const { return health_; }
	[[nodiscard]] bool IsAlive() const { return health_ > 0; }
	[[nodiscard]] Team GetTeam() const { return team_; }
	[[nodiscard]] Vector3 GetForward() const;

	private:
	// コントローラーだけが飛行状態を直接更新できるようにする。
	friend class PlayerController;
	friend class EnemyController;
	friend class GameEditor;

	// ===== 表示データ =====

	Object3d object_; // 機体モデル

	// ===== 飛行状態 =====

	Vector3 position_{};  // ワールド座標
	float yaw_ = 0.0f;    // 水平方向の向き
	float pitch_ = 0.0f;  // 機首の上下角度
	float roll_ = 0.0f;   // 機体の傾き
	float speed_ = 55.0f; // 前進速度

	// ===== 戦闘状態 =====

	int health_ = 3;           // 残り耐久値
	Team team_ = Team::Player; // ミサイルの攻撃対象判定に使う所属
};
