#pragma once

#include "engine/math/MathTypes.h"

class Camera;
class Fighter;

/// <summary>
/// 距離、機首方向、画面内判定を使ってロックオン状態を管理するクラス
/// </summary>
class LockOnSystem final {
  public:
	/// <summary>
	/// ロックオン条件を評価し、ターゲットの画面座標を更新する
	/// </summary>
	bool Update(const Fighter &source, const Fighter &target, const Camera &camera);

	/// <summary>
	/// ロックオン状態を解除する
	/// </summary>
	void Reset();

	/// <summary>
	/// 現在ロックオンしているかを取得する
	/// </summary>
	[[nodiscard]] bool IsLockedOn() const { return lockedOn_; }

	/// <summary>
	/// ロックマーカーを表示する画面座標を取得する
	/// </summary>
	[[nodiscard]] const Vector2 &GetScreenPosition() const { return screenPosition_; }

  private:
	bool lockedOn_ = false;    // ロックオン条件を満たしているか
	Vector2 screenPosition_{}; // ターゲットを投影した画面座標
};
