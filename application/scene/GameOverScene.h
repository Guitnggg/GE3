#pragma once

#include "engine/scene/Scene.h"

namespace GameApp {

/// <summary>
/// ゲーム失敗後の結果画面を管理するシーン。
/// </summary>
class GameOverScene final : public Scene {
  private:
	/// <summary>
	/// オブジェクトの更新処理を実行する
	/// </summary>
	void OnUpdate() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void OnDraw() override;
};

} // namespace GameApp
