#pragma once

#include "engine/core/Engine.h"

/// <summary>
/// Engineのライフサイクルへ接続する、最小構成のアプリケーション実装。
/// </summary>
class MyGame : public Engine {
  public:
	MyGame();
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
	/// 空のフレームとデバッグUIを描画する。
	/// </summary>
	void OnDraw() override;

	/// <summary>
	/// ゲーム固有の終了処理。
	/// </summary>
	void OnFinalize() override;
};
