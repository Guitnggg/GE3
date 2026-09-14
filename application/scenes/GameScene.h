#pragma once

#include "application/characters/Enemy.h"
#include "application/characters/Player.h"
#include "application/editor/GameParameterEditor.h"
#include "engine/3D/object/Object3d.h"
#include "engine/scene/IScene.h"
#include <cstdint>
#include <memory>
#include <vector>

class Model;

/// <summary>
/// 3Dレールシューティング全体の進行と得点を管理する。
/// </summary>
class GameScene final : public IScene {
public:
	/// <summary>
	/// シーンが所有するゲーム要素と参照を解放する。
	/// </summary>
	~GameScene() override;

	/// <summary>
	/// 共通機能を受け取り、プレイヤー、モデル、レールを生成する。
	/// </summary>
	/// <param name="context">Frameworkが所有するゲーム共通機能</param>
	void Initialize(const SceneContext& context) override;

	/// <summary>
	/// 毎フレーム呼び出されるメソッド。ゲームの状態を更新するためにオーバーライドされます。
	/// </summary>
	void Update() override;

	/// <summary>
	/// 固定更新時に呼び出されるメソッド。物理演算や時間に依存する更新処理を一定間隔で行うためにオーバーライドされます。
	/// </summary>
	void FixedUpdate() override;

	/// <summary>
	/// 3Dオブジェクトと照準を描画する。
	/// </summary>
	void Draw() override;

	/// <summary>
	/// シーンが所有するオブジェクトと参照を解放する。
	/// </summary>
	void Finalize() override;

private:
	/// <summary>
	/// レールマーカーを作成する。
	/// </summary>
	/// <param name="x">マーカーを配置するX座標</param>
	/// <param name="z">マーカーを配置するZ座標</param>
	/// <returns>初期化済みの3Dオブジェクト</returns>
	std::unique_ptr<Object3d> CreateRailMarker(float x, float z);

	/// <summary>
	/// ゲーム要素とスコアを開始状態へ戻す。
	/// </summary>
	void ResetGame();

	/// <summary>
	/// 現在の難易度パラメータに従って敵を1体生成する。
	/// </summary>
	void SpawnEnemy();

	/// <summary>
	/// プレイヤーの射線に最も近い敵を撃破する。
	/// </summary>
	void Shoot();

	/// <summary>
	/// レールマーカーを再配置し、描画行列を更新する。
	/// </summary>
	void UpdateRail();

	/// <summary>
	/// カメラを通過した敵を削除し、プレイヤーへダメージを与える。
	/// </summary>
	void RemovePassedEnemies();

private:
	SceneContext context_{};                              // Frameworkが所有する共通機能への非所有参照
	std::shared_ptr<Model> sphereModel_;                  // 敵とレールマーカーで共有する球モデル
	std::unique_ptr<Player> player_;                      // カメラ、照準、ライフを持つプレイヤー
	std::vector<std::unique_ptr<Enemy>> enemies_;         // 現在出現している敵
	std::vector<std::unique_ptr<Object3d>> railMarkers_;  // 自動前進を視覚化する左右の目印
	GameParameters parameters_{};                         // 実行中に調整可能なゲーム設定
	GameParameterEditor parameterEditor_{};               // ゲーム設定を操作するデバッグUI
	uint32_t texture_ = 0;                                // 球と照準に使用するテクスチャ番号
	uint32_t score_ = 0;                                  // 撃破した敵の数
	uint32_t spawnSequence_ = 0;                          // 敵配置パターンを選択する通し番号
	float enemySpawnTimer_ = 0.0f;                        // 次の敵生成までの残り秒数
	bool gameOver_ = false;                               // ゲームオーバー状態
	bool initialized_ = false;                            // 多重初期化を防ぐ状態
};
