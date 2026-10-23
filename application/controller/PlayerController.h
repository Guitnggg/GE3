#pragma once

class Fighter;
class Input;

/// <summary>
/// 入力状態をプレイヤー機の飛行操作へ変換するクラス
/// </summary>
class PlayerController final {
  public:
	/// <summary>旋回、姿勢、速度、移動位置を更新する</summary>
	void Update(Fighter &fighter, Input &input, float deltaTime) const;
};
