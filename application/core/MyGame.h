#pragma once

#include "engine/core/Engine.h"

/// <summary>
/// エンジンを起動する最小構成のアプリケーションクラス。
/// </summary>
class MyGame : public Engine {
  public:
	MyGame();
	~MyGame() override;
	MyGame(const MyGame &) = delete;
	MyGame &operator=(const MyGame &) = delete;

	/// <summary>
	/// エンジン共通機能を初期化する。
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// エンジン共通処理を更新する。
	/// </summary>
	void Update() override;

	/// <summary>
	/// 固定間隔ロジックを更新する。
	/// </summary>
	void FixedUpdate() override;

	/// <summary>
	/// 空のフレームとデバッグUIを描画する。
	/// </summary>
	void Draw() override;

	/// <summary>
	/// エンジン共通機能を終了する。
	/// </summary>
	void Finalize() override;

};
