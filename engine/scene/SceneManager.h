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
	/// <summary>
	/// 積み重ねたシーンが下層へ処理を通すか指定する。
	/// </summary>
	struct LayerOptions {
		bool updateBelow = false; // 下にあるシーンも更新するか
		bool drawBelow = true;    // 下にあるシーンも描画するか
	};

	/// <summary>
	/// シーンを持たない未初期化状態で生成する。
	/// </summary>
	SceneManager() = default;

	/// <summary>
	/// 保持しているシーンを終了して破棄する。
	/// </summary>
	~SceneManager();

	/// <summary>
	/// シーンの所有権を一意に保つため、コピーを禁止する。
	/// </summary>
	SceneManager(const SceneManager &) = delete;

	/// <summary>
	/// シーンの所有権を一意に保つため、コピー代入を禁止する。
	/// </summary>
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

	/// <summary>
	/// 現在のシーンを一時停止し、その上へ新しいシーンを積む。
	/// </summary>
	void PushScene(std::unique_ptr<IScene> scene, LayerOptions options = {});

	/// <summary>
	/// 最上位シーンを終了し、その下のシーンへ戻る。
	/// </summary>
	void PopScene();

	/// <summary>
	/// 指定した型のシーンを生成し、次の更新開始時の置き換えを予約する。
	/// </summary>
	/// <typeparam name="T">生成するIScene派生型</typeparam>
	/// <param name="args">シーンのコンストラクターへ渡す引数</param>
	template <typename T, typename... Args>
	    requires std::derived_from<T, IScene>
	void ChangeScene(Args &&...args) {
		ChangeScene(std::make_unique<T>(std::forward<Args>(args)...));
	}

	/// <summary>
	/// 指定した型のシーンを生成し、現在のシーン上へ積む操作を予約する。
	/// </summary>
	/// <typeparam name="T">生成するIScene派生型</typeparam>
	/// <param name="options">下層シーンの更新と描画に関する設定</param>
	/// <param name="args">シーンのコンストラクターへ渡す引数</param>
	template <typename T, typename... Args>
	    requires std::derived_from<T, IScene>
	void PushScene(LayerOptions options, Args &&...args) {
		PushScene(std::make_unique<T>(std::forward<Args>(args)...), options);
	}

	/// <summary>
	/// 現在積まれているシーン数を取得する。
	/// </summary>
	[[nodiscard]] size_t GetSceneCount() const {
		return stack_.size();
	}

	/// <summary>
	/// 最上位のシーンを取得する。
	/// </summary>
	/// <returns>シーンがない場合はnullptr</returns>
	[[nodiscard]] IScene *GetTopScene();

	/// <summary>
	/// 最上位のシーンを読み取り専用で取得する。
	/// </summary>
	/// <returns>シーンがない場合はnullptr</returns>
	[[nodiscard]] const IScene *GetTopScene() const;

  private:
	enum class OperationType {
		Replace,
		Push,
		Pop
	};
	/// <summary>
	/// 実行中のシーン1層分と、その下層へ適用する設定を保持する。
	/// </summary>
	struct SceneEntry {
		std::unique_ptr<IScene> scene; // SceneManagerが所有するシーン
		LayerOptions options{};        // 下層シーンへ適用する実行条件
	};

	/// <summary>
	/// 安全なタイミングまで適用を遅らせるシーン操作を保持する。
	/// </summary>
	struct PendingOperation {
		OperationType type = OperationType::Pop; // 次フレーム先頭で行う操作
		std::unique_ptr<IScene> scene;           // ReplaceまたはPushするシーン
		LayerOptions options{};                  // Push時に使用するレイヤー設定
	};

	/// <summary>
	/// 待機中のシーン操作を登録順に適用する。
	/// </summary>
	void ApplyPendingOperations();

	/// <summary>
	/// 現在の全シーンを終了し、指定したシーンへ置き換える。
	/// </summary>
	void ApplyReplace(std::unique_ptr<IScene> scene);

	/// <summary>
	/// 現在の最上位シーンを一時停止し、新しいシーンを積む。
	/// </summary>
	void ApplyPush(std::unique_ptr<IScene> scene, LayerOptions options);

	/// <summary>
	/// 最上位シーンを終了し、その下のシーンを再開する。
	/// </summary>
	void ApplyPop();

	/// <summary>
	/// このフレームで更新を開始する最下層シーンの番号を求める。
	/// </summary>
	[[nodiscard]] size_t FindFirstUpdateScene() const;

	/// <summary>
	/// このフレームで描画を開始する最下層シーンの番号を求める。
	/// </summary>
	[[nodiscard]] size_t FindFirstDrawScene() const;

	/// <summary>
	/// SceneManagerが初期化済みであることを検証する。
	/// </summary>
	void EnsureInitialized() const;

	SceneContext context_{};                         // 全シーンへ渡すエンジン機能
	std::vector<SceneEntry> stack_;                  // 下層から上層へ並ぶ実行中シーン
	std::deque<PendingOperation> pendingOperations_; // 適用待ちの安全なシーン操作
	bool initialized_ = false;                       // 初期シーンを開始済みか
};
