#pragma once

#include "application/characters/Player.h"
#include "application/editor/GameParameterEditor.h"
#include "application/effects/EnemyDeathEffect.h"
#include "application/effects/MissileTrailEffect.h"
#include "application/effects/PlayerEngineEffect.h"
#include "application/environment/StageEnvironment.h"
#include "application/managers/EnemyManager.h"
#include "application/managers/WeaponManager.h"
#include "engine/input/Input.h"
#include "engine/scene/IScene.h"

#include <cstdint>
#include <memory>

class Model;

/// <summary>3Dレールシューティング全体の進行と得点を管理する。</summary>
class GameScene final : public IScene {
public:
	~GameScene() override;
	void Initialize(const SceneContext& context) override;
	void Update() override;
	void FixedUpdate() override;
	void Draw() override;
	void Finalize() override;

private:
	void InitializeInputActions();
	void InitializeGameObjects();
	void ResetGame();
	void UpdateGameplay(float deltaTime, bool acceptFireInput);
	void UpdateFrameSystems(float deltaTime);
	void DrawDebugUi();

	SceneContext context_{};

	// 読み込み済みリソース
	std::shared_ptr<Model> sphereModel_;
	std::shared_ptr<Model> playerModel_;
	std::shared_ptr<Model> missileModel_;
	uint32_t texture_ = 0;
	uint32_t lockOnTexture_ = 0;
	uint32_t particleTexture_ = 0;

	// シーンを構成するゲーム要素
	std::unique_ptr<Player> player_;
	std::unique_ptr<StageEnvironment> stageEnvironment_;
	std::unique_ptr<PlayerEngineEffect> playerEngineEffect_;
	std::unique_ptr<EnemyDeathEffect> enemyDeathEffect_;
	std::unique_ptr<MissileTrailEffect> missileTrailEffect_;
	EnemyManager enemyManager_{};
	WeaponManager weaponManager_{};

	// ゲーム設定と入力
	GameParameters parameters_{};
	GameParameterEditor parameterEditor_{};
	InputActionId moveAction_ = kInvalidInputActionId;
	InputActionId shootAction_ = kInvalidInputActionId;
	InputActionId lockOnAction_ = kInvalidInputActionId;
	InputActionId restartAction_ = kInvalidInputActionId;

	uint32_t score_ = 0;
	bool gameOver_ = false;
	bool initialized_ = false;
};
