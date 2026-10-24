#pragma once

#include "engine/scene/Scene.h"

namespace GameApp {

/// <summary>
/// ゲーム開始前のタイトル画面を管理するシーン。
/// </summary>
class TitleScene final : public Scene {
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
