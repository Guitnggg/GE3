#pragma once

#include "engine/scene/SceneContext.h"

/// <summary>
/// すべてのゲームシーンが実装する共通インターフェース。
/// </summary>
class IScene {
public:
	virtual ~IScene() = default;

	virtual void Initialize(const SceneContext& context) = 0;
	virtual void Update() = 0;
	virtual void FixedUpdate() = 0;
	virtual void Draw() = 0;
	virtual void Finalize() = 0;
};
