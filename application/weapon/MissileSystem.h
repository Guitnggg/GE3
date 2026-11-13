#pragma once

#include "application/effect/CombatEffectSystem.h"
#include "application/weapon/Missile.h"

#include <memory>
#include <vector>

class AssetManager;
class Camera;
class Fighter;
class Input;
class Model;
class GameEditor;
struct SceneContext;

/// <summary>
/// ミサイルの発射、誘導、命中判定、破棄をまとめて管理するクラス
/// </summary>
class MissileSystem final {
  public:
	/// <summary>
	/// ミサイルモデルと戦闘演出を初期化する
	/// </summary>
	void Initialize(const SceneContext &context);

	/// <summary>
	/// 全ミサイルと発射待ち時間を初期状態へ戻す
	/// </summary>
	void Reset();

	/// <summary>
	/// 発射入力、敵の自動発射、誘導、命中判定を更新する
	/// </summary>
	/// <returns>いずれかの機体が撃墜された場合はtrue</returns>
	bool Update(Input &input, Fighter &player, Fighter &enemy, bool lockedOn, float deltaTime);

	/// <summary>
	/// 全ミサイルの表示状態を更新する
	/// </summary>
	void UpdateVisuals(const Camera &camera);

	/// <summary>
	/// 全ミサイルと戦闘パーティクルを描画する
	/// </summary>
	void Draw(const Camera &camera, float deltaTime);

  private:
	friend class GameEditor;
	/// <summary>
	/// 指定した機体の前方からミサイルを生成する
	/// </summary>
	void Spawn(const Fighter &owner);

	const SceneContext *context_ = nullptr;          // ミサイル生成に必要なエンジン機能
	std::shared_ptr<Model> model_;                   // 全ミサイルで共有するモデル
	std::vector<std::unique_ptr<Missile>> missiles_; // 飛行中のミサイル
	CombatEffectSystem effects_;                     // 航跡と爆発の描画
	float playerCooldown_ = 0.0f;                    // プレイヤーの再発射待ち時間
	float enemyCooldown_ = 0.0f;                     // 敵の再発射待ち時間
};
