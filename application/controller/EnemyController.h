#pragma once

class Fighter;

/// <summary>
/// 敵機をターゲットへ追尾させる簡易AIコントローラー
/// </summary>
class EnemyController final {
  public:
	/// <summary>
	/// ターゲット方向へ機首を向けながら敵機を前進させる
	/// </summary>
	void Update(Fighter &fighter, const Fighter &target, float deltaTime) const;
};
