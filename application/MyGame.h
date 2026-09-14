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

	void Initialize() override;
	void Update() override;
	void FixedUpdate() override;
	void Draw() override;
	void Finalize() override;

private:
	std::unique_ptr<SceneManager> sceneManager_;
	bool initialized_ = false;
};
