#pragma once

#include "application/characters/Enemy.h"
#include "application/characters/Player.h"
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
	/// 
	/// </summary>
	~GameScene() override;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="context"></param>
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
	/// 
	/// </summary>
	void Draw() override;

	/// <summary>
	/// 
	/// </summary>
	void Finalize() override;

private:
	/// <summary>
	/// レールマーカーを作成する。
	/// </summary>
	/// <param name="x"></param>
	/// <param name="z"></param>
	/// <returns></returns>
	std::unique_ptr<Object3d> CreateRailMarker(float x, float z);

	/// <summary>
	/// 
	/// </summary>
	void ResetGame();

	/// <summary>
	/// 
	/// </summary>
	void SpawnEnemy();

	/// <summary>
	/// 
	/// </summary>
	void Shoot();

	/// <summary>
	/// 
	/// </summary>
	void UpdateRail();

	/// <summary>
	/// 
	/// </summary>
	void RemovePassedEnemies();

private:
	SceneContext context_{};
	std::shared_ptr<Model> sphereModel_;
	std::unique_ptr<Player> player_;
	std::vector<std::unique_ptr<Enemy>> enemies_;
	std::vector<std::unique_ptr<Object3d>> railMarkers_;
	uint32_t texture_ = 0;
	uint32_t score_ = 0;
	uint32_t spawnSequence_ = 0;
	float enemySpawnTimer_ = 0.0f;
	bool gameOver_ = false;
	bool initialized_ = false;
};
