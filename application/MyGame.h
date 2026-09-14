#pragma once

#include "application/Framework.h"

#include <memory>

class SceneManager;

/// <summary>
/// エンジン共通処理とゲームシーンを接続するアプリケーションクラス。
/// </summary>
class MyGame : public Framework {
public:
	MyGame();
	~MyGame() override;
	MyGame(const MyGame&) = delete;
	MyGame& operator=(const MyGame&) = delete;

	/// <summary>
	/// エンジン共通機能と最初のゲームシーンを初期化する。
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// エンジン共通処理に続けて、現在のゲームシーンを更新する。
	/// </summary>
	void Update() override;

	/// <summary>
	/// 現在のゲームシーンの固定間隔ロジックを更新する。
	/// </summary>
	void FixedUpdate() override;

	/// <summary>
	/// 現在のゲームシーンとデバッグUIを描画する。
	/// </summary>
	void Draw() override;

	/// <summary>
	/// ゲームシーンとエンジン共通機能を終了する。
	/// </summary>
	void Finalize() override;

private:
	std::unique_ptr<SceneManager> sceneManager_;
	bool initialized_ = false;
};
