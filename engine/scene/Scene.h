#pragma once

#include "engine/object/GameObject.h"
#include "engine/scene/IScene.h"

#include <concepts>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/// <summary>
/// GameObjectの所有と標準的な更新順序を提供するシーン基底クラス。
/// 派生シーンはOn系フックへゲーム固有処理だけを記述する。
/// </summary>
class Scene : public IScene {
  public:
	Scene() = default;
	~Scene() override = default;
	Scene(const Scene &) = delete;
	Scene &operator=(const Scene &) = delete;

	/// <summary>
	/// 与えられたシーンコンテキストを用いてシーンの初期化を行う。
	/// </summary>
	/// <param name="context">初期化に使用するシーンのコンテキスト</param>
	void Initialize(const SceneContext &context) final;

	/// <summary>
	/// 更新処理を行うメンバ関数。
	/// </summary>
	void Update() final;

	/// <summary>
	/// 固定間隔で実行される更新処理を行うメソッド。
	/// </summary>
	void FixedUpdate() final;

	/// <summary>
	/// 描画を行うメンバ関数。
	/// </summary>
	void Draw() final;

	/// <summary>
	/// オブジェクトの終了処理を行う、派生クラスでオーバーライドできない（final指定された）メンバ関数。
	/// </summary>
	void Finalize() final;

	/// <summary>
	/// シーンが所有するGameObjectを生成し、参照を返す。
	/// </summary>
	GameObject &CreateGameObject(std::string name = "GameObject");

	/// <summary>
	/// GameObjectを現在処理の終端での破棄対象にする。
	/// 要求直後からコールバックを止め、コンテナからの削除だけを安全な境界まで遅らせる。
	/// </summary>
	void DestroyGameObject(GameObject &gameObject);

	[[nodiscard]] const SceneContext &GetContext() const;

  protected:
	/// <summary>
	/// 初期化時に呼び出される仮想メソッド。
	/// </summary>
	virtual void OnInitialize() {}

	/// <summary>
	/// 更新時に呼び出される仮想メソッド。オーバーライドして更新処理を実装するための関数。
	/// </summary>
	virtual void OnUpdate() {}

	/// <summary>
	/// 固定時間ステップ（固定更新）ごとに呼び出される仮想メソッド。
	/// </summary>
	virtual void OnFixedUpdate() {}

	/// <summary>
	/// 描画処理を行う仮想メソッド。
	/// </summary>
	virtual void OnDraw() {}

	/// <summary>
	/// オブジェクトの最終化時に呼び出される仮想メソッド。
	/// </summary>
	virtual void OnFinalize() {}

  private:
	/// <summary>
	/// 破棄済みオブジェクトの処理（フラッシュ）を行います。
	/// </summary>
	void FlushDestroyedObjects();

	SceneContext context_{};                                  // Engineが所有するサービスへの非所有参照
	std::vector<std::unique_ptr<GameObject>> gameObjects_;    // シーンと寿命を共有するGameObject
	std::vector<GameObject *> pendingDestroy_;                // 安全なフレーム境界まで遅延する破棄要求
	bool initialized_ = false;                                // ライフサイクルの多重実行を防ぐ状態
};
