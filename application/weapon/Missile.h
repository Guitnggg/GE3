#pragma once

#include "application/combat/Team.h"
#include "engine/3D/object/Object3d.h"

#include <memory>

class Camera;
class Model;
class Object3dCommon;
class TextureManager;

/// <summary>
/// 1発のミサイルが持つ位置、速度、寿命、表示モデルを管理するクラス
/// </summary>
class Missile final {
  public:
	/// <summary>
	/// 発射時の状態と表示モデルを初期化する
	/// </summary>
	void Initialize(Object3dCommon *object3dCommon, TextureManager *textureManager,
	                const std::shared_ptr<Model> &model, const Vector3 &position,
	                const Vector3 &velocity, Team ownerTeam);

	/// <summary>
	/// 位置と進行方向を3Dモデルへ反映する
	/// </summary>
	void UpdateVisual(const Camera &camera);

	/// <summary>
	/// ミサイルモデルを描画する
	/// </summary>
	void Draw() const;

	// ===== ミサイル状態の参照・操作 =====
	
	[[nodiscard]] const Vector3 &GetPosition() const { return position_; }
	[[nodiscard]] const Vector3 &GetVelocity() const { return velocity_; }
	[[nodiscard]] float GetLife() const { return life_; }
	[[nodiscard]] Team GetOwnerTeam() const { return ownerTeam_; }
	void SetVelocity(const Vector3 &velocity) { velocity_ = velocity; }
	void Advance(float deltaTime);
	void Expire() { life_ = 99.0f; }

  private:
	Object3d object_;               // ミサイルの表示モデル
	Vector3 position_{};            // 現在のワールド座標
	Vector3 velocity_{};            // 1秒あたりの移動量
	Team ownerTeam_ = Team::Player; // 発射した機体の所属
	float life_ = 0.0f;             // 発射後の経過時間
};
