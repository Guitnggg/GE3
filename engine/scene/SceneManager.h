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

	/// <summary>
	/// シーン共通機能を保持し、最初のシーンを初期化する。
	/// </summary>
	void Initialize(const SceneContext& context, std::unique_ptr<IScene> initialScene);

	/// <summary>
	/// 予約されたシーン切り替えを適用し、現在のシーンを更新する。
	/// </summary>
	void Update();

	/// <summary>
	/// 現在のシーンの固定間隔ロジックを更新する。
	/// </summary>
	void FixedUpdate();

	/// <summary>
	/// 現在のシーンを描画する。
	/// </summary>
	void Draw();

	/// <summary>
	/// 現在および切り替え待ちのシーンを終了する。
	/// </summary>
	void Finalize();

	/// <summary>
	/// 次のUpdate開始時に切り替えるシーンを予約する。
	/// </summary>
	void ChangeScene(std::unique_ptr<IScene> nextScene);

private:
	/// <summary>
	/// 切り替え待ちのシーンがあれば、現在のシーンと入れ替える。
	/// </summary>
	void ApplyPendingScene();

	SceneContext context_{};
	std::unique_ptr<IScene> currentScene_;
	std::unique_ptr<IScene> pendingScene_;
	bool initialized_ = false;
};
