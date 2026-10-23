#pragma once

#include "application/actor/Fighter.h"
#include "application/camera/CombatCamera.h"
#include "application/combat/LockOnSystem.h"
#include "application/controller/EnemyController.h"
#include "application/controller/PlayerController.h"
#include "application/ui/CombatHud.h"
#include "application/weapon/MissileSystem.h"
#include "engine/scene/Scene.h"

#include <memory>

class Model;

/// <summary>
/// 1対1の空戦に必要な機体、武器、カメラ、HUDを統括するゲームシーン
/// </summary>
class AirCombatScene final : public Scene {
  protected:
	/// <summary>
	/// 空戦で使用する各システムとアセットを初期化する
	/// </summary>
	void OnInitialize() override;

	/// <summary>
	/// 入力、AI、戦闘、カメラを1フレーム分更新する
	/// </summary>
	void OnUpdate() override;

	/// <summary>
	/// 機体、ミサイル、演出、HUDを描画する
	/// </summary>
	void OnDraw() override;

  private:
	/// <summary>
	/// 両機と戦闘システムを開始状態へ戻す
	/// </summary>
	void ResetBattle();

	/// <summary>
	/// シミュレーション結果を各3Dモデルへ反映する
	/// </summary>
	void UpdateVisuals();

	// ===== 機体 =====

	std::shared_ptr<Model> fighterModel_; // 両機で共有する戦闘機モデル
	Fighter player_;                      // プレイヤー機
	Fighter enemy_;                       // 敵機

	// ===== 操作・戦闘システム =====

	PlayerController playerController_;
	EnemyController enemyController_;
	LockOnSystem lockOnSystem_;
	MissileSystem missiles_;

	// ===== 表示システム =====

	CombatCamera camera_;
	CombatHud hud_;

	// ===== シーン状態 =====

	bool lockedOn_ = false;    // 敵機をロックオンしているか
	bool battleEnded_ = false; // どちらかの機体が撃墜されたか
};
