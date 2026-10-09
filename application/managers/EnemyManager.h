#pragma once

#include "application/characters/Enemy.h"
#include "engine/math/MathTypes.h"

#include <cstdint>
#include <memory>
#include <vector>

class Camera;
class CollisionWorld;
struct GameParameters;
class Model;
class Object3dCommon;
class SpriteCommon;
class TextureManager;

/// <summary>
/// 敵の生成、所有、更新、検索、削除をまとめて管理する。
/// </summary>
class EnemyManager final {
  public:
	/// <summary>
	/// 所有している敵と衝突判定を安全に破棄する。
	/// </summary>
	~EnemyManager();

	/// <summary>
	/// 敵の生成に必要なサービス、共有モデル、ロックオン画像を設定する。
	/// </summary>
	void Initialize(CollisionWorld *collisionWorld,
	                SpriteCommon *spriteCommon,
	                Object3dCommon *object3dCommon,
	                TextureManager *textureManager,
	                const std::shared_ptr<Model> &model,
	                uint32_t lockOnTexture);
	/// <summary>
	/// すべての敵を破棄し、生成状態をゲーム開始時へ戻す。
	/// </summary>
	void Reset();

	/// <summary>
	/// 全敵の移動、回転、衝突判定、ロックオン表示を更新する。
	/// </summary>
	void Update(const Camera &camera, float deltaTime, float rotationSpeed);

	/// <summary>
	/// 難易度設定と得点に従い、必要なタイミングで敵を生成する。
	/// </summary>
	void UpdateSpawning(float deltaTime, float cameraZ, const GameParameters &parameters, uint32_t score);
	
	/// <summary>
	/// 生存している全敵を描画する。
	/// </summary>
	void Draw() const;
	
	/// <summary>
	/// ロック中の敵に照準マーカーを描画する。
	/// </summary>
	void DrawLockOnMarkers() const;

	/// <summary>
	/// 指定IDの敵を検索する。見つからない場合はnullptrを返す。
	/// </summary>
	Enemy *Find(uint64_t id) const;
	
	/// <summary>
	/// 除外ID以外から、指定位置に最も近いロック可能な敵を検索する。
	/// </summary>
	Enemy *FindNearestLockTarget(const Vector3 &origin, const std::vector<uint64_t> &excludedIds) const;
	
	/// <summary>
	/// 指定ID群と一致する敵のロック表示を有効にする。
	/// </summary>
	void SetLockedEnemies(const std::vector<uint64_t> &ids);
	
	/// <summary>
	/// 指定IDの敵を削除する。削除できた場合はtrueを返す。
	/// </summary>
	bool Remove(uint64_t id);
	
	/// <summary>
	/// プレイヤー後方へ通過した敵を削除し、そのID一覧を返す。
	/// </summary>
	std::vector<uint64_t> RemovePassed(float playerZ);

  private:
	/// <summary>
	/// 現在のカメラ位置より前方へ敵を1体生成する。
	/// </summary>
	void Spawn(float cameraZ, const GameParameters &parameters);

	Object3dCommon *object3dCommon_ = nullptr;
	SpriteCommon *spriteCommon_ = nullptr;
	CollisionWorld *collisionWorld_ = nullptr;
	TextureManager *textureManager_ = nullptr;
	std::shared_ptr<Model> model_;
	uint32_t lockOnTexture_ = 0;
	std::vector<std::unique_ptr<Enemy>> enemies_;
	uint32_t spawnSequence_ = 0; // 出現位置を決める決定的乱数の通し番号
	uint64_t nextEnemyId_ = 1;   // 次に割り当てる一意な敵ID
	float spawnTimer_ = 0.0f;    // 次の出現までの残り秒数
};
