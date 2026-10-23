#pragma once

#include "engine/3D/camera/Camera.h"

class Fighter;

/// <summary>
/// プレイヤー機を後方から追従する空戦用カメラ
/// </summary>
class CombatCamera final {
  public:
	/// <summary>
	/// 投影設定と描画距離を初期化する
	/// </summary>
	void Initialize();

	/// <summary>
	/// プレイヤーの現在位置へカメラを即座に合わせる
	/// </summary>
	void Reset(const Fighter &player);

	/// <summary>
	/// プレイヤーの後方へ滑らかに追従する
	/// </summary>
	void Update(const Fighter &player, float deltaTime);

	/// <summary>
	/// 描画に使用するカメラを取得する
	/// </summary>
	[[nodiscard]] const Camera &GetCamera() const { return camera_; }

  private:
	Camera camera_;                          // エンジンへ渡す描画カメラ
	Vector3 position_{0.0f, 36.0f, -105.0f}; // 補間中のカメラ座標
};
