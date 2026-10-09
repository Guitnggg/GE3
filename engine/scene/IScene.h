#pragma once

#include "engine/scene/SceneContext.h"

/// <summary>
/// SceneManagerが統一した手順で操作するシーンの基底インターフェース。
/// </summary>
class IScene {
  public:
	virtual ~IScene() = default;

	/// <summary>
	/// シーンが利用するエンジン共通機能を受け取り、初期状態を構築する。
	/// </summary>
	virtual void Initialize(const SceneContext &context) = 0;

	/// <summary>
	/// 上に別シーンが重なり、このシーンの操作が一時停止するときに呼ばれる。
	/// </summary>
	virtual void OnPause() {}

	/// <summary>
	/// 上のシーンが閉じられ、このシーンへ操作が戻るときに呼ばれる。
	/// </summary>
	virtual void OnResume() {}

	/// <summary>
	/// 入力や時間に応じて、シーンの状態を毎フレーム更新する。
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 一定時間間隔でシーンのゲームロジックを更新する。
	/// </summary>
	virtual void FixedUpdate() = 0;

	/// <summary>
	/// シーンが所有する要素を描画する。
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// シーンが確保したリソースを解放する。
	/// </summary>
	virtual void Finalize() = 0;
};
