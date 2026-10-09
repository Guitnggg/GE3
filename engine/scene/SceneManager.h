#pragma once

#include "engine/scene/IScene.h"
#include "engine/scene/SceneContext.h"

#include <concepts>
#include <cstddef>
#include <deque>
#include <memory>
#include <utility>
#include <vector>

/// <summary>
/// 現在のシーンのライフサイクルと安全な切り替えを管理するクラス。
/// </summary>
class SceneManager {
  public:
	struct LayerOptions {
		bool updateBelow = false;
		bool drawBelow = true;
	};

	SceneManager() = default;
	~SceneManager();
	SceneManager(const SceneManager &) = delete;
	SceneManager &operator=(const SceneManager &) = delete;

	/// <summary>
	/// シーン共通機能を保持し、最初のシーンを初期化する。
	/// </summary>
	void Initialize(const SceneContext &context, std::unique_ptr<IScene> initialScene);

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

	/// <summary>現在のシーンを一時停止し、その上へ新しいシーンを積む。</summary>
	void PushScene(std::unique_ptr<IScene> scene, LayerOptions options = {});

	/// <summary>最上位シーンを終了し、その下のシーンへ戻る。</summary>
	void PopScene();

	template <typename T, typename... Args>
		requires std::derived_from<T, IScene>
	void ChangeScene(Args &&...args) {
		ChangeScene(std::make_unique<T>(std::forward<Args>(args)...));
	}

	template <typename T, typename... Args>
		requires std::derived_from<T, IScene>
	void PushScene(LayerOptions options, Args &&...args) {
		PushScene(std::make_unique<T>(std::forward<Args>(args)...), options);
	}

	[[nodiscard]] size_t GetSceneCount() const {
		return stack_.size();
	}
	[[nodiscard]] IScene *GetTopScene();
	[[nodiscard]] const IScene *GetTopScene() const;

  private:
	enum class OperationType { Replace, Push, Pop };
	struct SceneEntry {
		std::unique_ptr<IScene> scene;
		LayerOptions options{};
	};
	struct PendingOperation {
		OperationType type = OperationType::Pop;
		std::unique_ptr<IScene> scene;
		LayerOptions options{};
	};

	void ApplyPendingOperations();
	void ApplyReplace(std::unique_ptr<IScene> scene);
	void ApplyPush(std::unique_ptr<IScene> scene, LayerOptions options);
	void ApplyPop();
	[[nodiscard]] size_t FindFirstUpdateScene() const;
	[[nodiscard]] size_t FindFirstDrawScene() const;
	void EnsureInitialized() const;

	SceneContext context_{};
	std::vector<SceneEntry> stack_;
	std::deque<PendingOperation> pendingOperations_;
	bool initialized_ = false;
};
