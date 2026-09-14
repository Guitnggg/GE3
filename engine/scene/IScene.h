#pragma once

#include "engine/scene/SceneContext.h"

/// <summary>
/// すべてのゲームシーンが実装する共通インターフェース。
/// </summary>
class IScene {
public:
	virtual ~IScene() = default;

	/// <summary>
	/// シーンが利用するエンジン共通機能を受け取り、初期状態を構築する。
	/// </summary>
	virtual void Initialize(const SceneContext& context) = 0;

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
