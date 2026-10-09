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

/// <summary>
/// 3Dレールシューティング全体の進行と得点を管理する。
/// </summary>
class GameScene final : public IScene {
  public:
	/// <summary>
	/// シーンが所有するゲーム要素を終了処理して破棄する。
	/// </summary>
	~GameScene() override;

	/// <summary>
	/// 共通サービスを受け取り、リソースとゲーム要素を生成する。
	/// </summary>
	void Initialize(const SceneContext &context) override;

	/// <summary>
	/// 入力と可変時間で進むゲーム処理を1フレーム更新する。
	/// </summary>
	void Update() override;

	/// <summary>
	/// 一定時間刻みが必要なゲーム処理を更新する。
	/// </summary>
	void FixedUpdate() override;

	/// <summary>
	/// 3Dオブジェクト、エフェクト、2D表示を適切な順序で描画する。
	/// </summary>
	void Draw() override;

	/// <summary>
	/// ゲーム要素と読み込み済みリソースを解放する。
	/// </summary>
	void Finalize() override;

  private:
	/// <summary>
	/// ゲーム操作に使用するアクションとデバイス割り当てを登録する。
	/// </summary>
	void InitializeInputActions();

	/// <summary>
	/// プレイヤー、敵、武器、演出、ステージを生成する。
	/// </summary>
	void InitializeGameObjects();

	/// <summary>
	/// 得点と全ゲーム要素を新しいプレイ状態へ戻す。
	/// </summary>
	void ResetGame();

	/// <summary>
	/// 操作受付中のプレイヤー、敵、武器、得点を更新する。
	/// </summary>
	void UpdateGameplay(float deltaTime, bool acceptFireInput);

	/// <summary>
	/// ゲーム状態に依存せず毎フレーム進む演出と環境を更新する。
	/// </summary>
	void UpdateFrameSystems(float deltaTime);

	/// <summary>
	/// デバッグビルド用のゲーム状態と調整UIを表示する。
	/// </summary>
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

	uint32_t score_ = 0;       // 現在のプレイで獲得した得点
	bool gameOver_ = false;    // 更新を停止してリスタート入力を待つ状態か
	bool initialized_ = false; // ゲーム要素を安全に利用できる状態か
};
