#pragma once

#include "engine/scene/SceneContext.h"

#include <memory>

class IScene;

/// <summary>
/// 現在のシーンのライフサイクルと安全な切り替えを管理するクラス。
/// </summary>
class SceneManager {
public:
	SceneManager() = default;
	~SceneManager();
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	void Initialize(const SceneContext& context, std::unique_ptr<IScene> initialScene);
	void Update();
	void FixedUpdate();
	void Draw();
	void Finalize();

	/// <summary>
	/// 次のUpdate開始時に切り替えるシーンを予約する。
	/// </summary>
	void ChangeScene(std::unique_ptr<IScene> nextScene);

private:
	void ApplyPendingScene();

	SceneContext context_{};
	std::unique_ptr<IScene> currentScene_;
	std::unique_ptr<IScene> pendingScene_;
	bool initialized_ = false;
};
