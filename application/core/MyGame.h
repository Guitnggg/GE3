#pragma once

#include <Windows.h>

#include "engine/core/EngineCore.h"

namespace GameApp {

/// <summary>
/// EngineCoreのライフサイクルへ接続する、最小構成のアプリケーション実装。
/// </summary>
class MyGame : public EngineCore {
  public:
	MyGame() = default;
	~MyGame() override = default;
	MyGame(const MyGame &) = delete;
	MyGame &operator=(const MyGame &) = delete;

	/// <summary>
	/// ゲーム固有の初期化処理。
	/// </summary>
	void OnInitialize() override;

	/// <summary>
	/// ゲーム固有の毎フレーム更新処理。
	/// </summary>
	void OnUpdate() override;

	/// <summary>
	/// 固定間隔ロジックを更新する。
	/// </summary>
	void OnFixedUpdate() override;

	/// <summary>
	/// ゲーム固有のワールド要素を描画する。
	/// </summary>
	void OnDrawWorld() override;

	/// <summary>
	/// ゲーム固有のUI要素を描画する。
	/// </summary>
	void OnDrawUI() override;
};

} // namespace GameApp
