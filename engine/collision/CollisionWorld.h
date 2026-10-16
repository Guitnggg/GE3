#pragma once

#include "engine/collision/Collision.h"

#include <cstdint>
#include <functional>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using CollisionLayer = uint32_t;

/// <summary>
/// CollisionWorldへ登録したColliderを識別するハンドル。
/// </summary>
struct ColliderHandle {
	uint64_t value = 0; // 登録済みColliderを一意に識別する番号

	/// <summary>
	/// 有効なCollider番号を保持しているか判定する。
	/// </summary>
	explicit operator bool() const noexcept {
		return value != 0;
	}

	/// <summary>
	/// 2つのハンドルが同じColliderを示すか比較する。
	/// </summary>
	bool operator==(const ColliderHandle &) const noexcept = default;
};

enum class CollisionEventType {
	Enter,
	Stay,
	Exit
};

/// <summary>
/// Collider自身から見た衝突イベント。
/// </summary>
struct CollisionEvent {
	CollisionEventType type = CollisionEventType::Enter; // 衝突開始・継続・終了のイベント種別
	ColliderHandle self{};                               // 通知を受け取る側のCollider
	ColliderHandle other{};                              // 衝突相手のCollider
	CollisionLayer otherLayer = 0;                       // 衝突相手が所属するレイヤー
	uint64_t otherUserData = 0;                          // 衝突相手に設定された利用者データ
	bool isTrigger = false;                              // 物理接触ではなくTriggerイベントか
};

using CollisionCallback = std::function<void(const CollisionEvent &)>;

/// <summary>
/// Sphere Colliderの登録情報。
/// </summary>
struct SphereColliderDesc {
	SphereCollider shape{};                                           // 登録する球の中心位置と半径
	CollisionLayer layer = 1;                                         // Collider自身が所属するレイヤー
	CollisionLayer mask = std::numeric_limits<CollisionLayer>::max(); // 衝突対象として許可するレイヤー集合
	uint64_t userData = 0;                                            // 利用者が自由に関連付ける識別値
	bool continuous = false;                                          // 前回位置からの連続判定を行うか
	bool isTrigger = false;                                           // 接触通知だけを行うTriggerか
	CollisionCallback callback{};                                     // 衝突状態が変化したときの通知先
};

/// <summary>
/// AABB Colliderの登録情報。
/// </summary>
struct AabbColliderDesc {
	AabbCollider shape{};                                             // 登録するAABBの最小点と最大点
	CollisionLayer layer = 1;                                         // Collider自身が所属するレイヤー
	CollisionLayer mask = std::numeric_limits<CollisionLayer>::max(); // 衝突対象として許可するレイヤー集合
	uint64_t userData = 0;                                            // 利用者が自由に関連付ける識別値
	bool isTrigger = false;                                           // 接触通知だけを行うTriggerか
	CollisionCallback callback{};                                     // 衝突状態が変化したときの通知先
};

/// <summary>
/// レイキャストの衝突情報を格納する構造体。
/// </summary>
struct RaycastHit {
	ColliderHandle collider{}; // 命中したCollider
	Vector3 point{};           // ワールド空間上の命中位置
	Vector3 normal{};          // 命中面から外側を向く法線
	float distance = 0.0f;     // レイ原点から命中位置までの距離
	CollisionLayer layer = 0;  // 命中したColliderの所属レイヤー
	uint64_t userData = 0;     // 命中したColliderの利用者データ
	bool isTrigger = false;    // 命中対象がTriggerか
};

/// <summary>
/// RaycastとOverlapで共通利用する対象フィルター。
/// </summary>
struct CollisionQueryFilter {
	CollisionLayer layerMask = std::numeric_limits<CollisionLayer>::max(); // 検索対象として許可するレイヤー集合
	bool includeTriggers = true;                                           // Triggerを検索結果へ含めるか
};

/// <summary>
/// Colliderの登録、レイヤーフィルタ、衝突状態、Raycastを一括管理する。
/// Updateは全Colliderの位置更新後、破棄処理前に1回呼び出す。
/// </summary>
class CollisionWorld final {
  public:
	/// <summary>
	/// 球形Colliderを登録し、操作に使用するハンドルを返す。
	/// </summary>
	ColliderHandle RegisterSphere(const SphereColliderDesc &desc);

	/// <summary>
	/// AABB Colliderを登録し、操作に使用するハンドルを返す。
	/// </summary>
	ColliderHandle RegisterAabb(const AabbColliderDesc &desc);

	/// <summary>
	/// 指定したColliderを登録解除する。
	/// </summary>
	void Unregister(ColliderHandle handle);

	/// <summary>
	/// すべてのColliderと衝突状態を登録解除する。
	/// </summary>
	void Clear();

	/// <summary>
	/// 登録済み球形Colliderの形状を更新する。
	/// </summary>
	void SetSphere(ColliderHandle handle, const SphereCollider &sphere);

	/// <summary>
	/// 登録済みAABB Colliderの形状を更新する。
	/// </summary>
	void SetAabb(ColliderHandle handle, const AabbCollider &aabb);

	/// <summary>
	/// 指定したColliderを衝突判定の対象にするか設定する。
	/// </summary>
	void SetEnabled(ColliderHandle handle, bool enabled);

	/// <summary>
	/// 指定したColliderの所属レイヤーと衝突対象マスクを設定する。
	/// </summary>
	void SetFilter(ColliderHandle handle, CollisionLayer layer, CollisionLayer mask);

	/// <summary>
	/// 指定したColliderをTriggerとして扱うか設定する。
	/// </summary>
	void SetTrigger(ColliderHandle handle, bool isTrigger);

	/// <summary>
	/// 指定したColliderがTriggerか取得する。
	/// </summary>
	[[nodiscard]] bool IsTrigger(ColliderHandle handle) const;

	/// <summary>
	/// 指定したハンドルのColliderが登録されているか判定する。
	/// </summary>
	bool IsRegistered(ColliderHandle handle) const;

	/// <summary>
	/// 現在の形状からEnter/Stay/Exitを計算し、登録されたCallbackへ通知する。
	/// </summary>
	void Update();

	/// <summary>
	/// 指定レイヤーに属するColliderのうち、最も近い命中を返す。
	/// </summary>
	bool Raycast(const Vector3 &origin,
	             const Vector3 &direction,
	             float maxDistance,
	             CollisionLayer layerMask,
	             RaycastHit &hit) const;

	/// <summary>
	/// フィルターに一致するすべてのRaycast結果を距離順で返す。
	/// </summary>
	[[nodiscard]] std::vector<RaycastHit> RaycastAll(const Vector3 &origin,
	                                                 const Vector3 &direction,
	                                                 float maxDistance,
	                                                 const CollisionQueryFilter &filter = {}) const;

	/// <summary>
	/// 指定球と重なるColliderを返す。
	/// </summary>
	[[nodiscard]] std::vector<ColliderHandle> OverlapSphere(const SphereCollider &sphere,
	                                                        const CollisionQueryFilter &filter = {}) const;

	/// <summary>
	/// 指定AABBと重なるColliderを返す。
	/// </summary>
	[[nodiscard]] std::vector<ColliderHandle> OverlapAabb(const AabbCollider &aabb,
	                                                      const CollisionQueryFilter &filter = {}) const;

	/// <summary>
	/// 現在登録されているCollider数を取得する。
	/// </summary>
	uint32_t GetColliderCount() const {
		return static_cast<uint32_t>(colliders_.size());
	}

  private:
	enum class ShapeType {
		Sphere,
		Aabb
	};
	/// <summary>
	/// CollisionWorldが保持する1つのColliderの内部状態。
	/// </summary>
	struct Collider {
		ShapeType type = ShapeType::Sphere;                               // 現在使用している衝突形状
		SphereCollider sphere{};                                          // 球形Colliderの現在形状
		SphereCollider previousSphere{};                                  // 連続判定に使用する前回の球形状
		AabbCollider aabb{};                                              // AABB Colliderの現在形状
		CollisionLayer layer = 1;                                         // Collider自身の所属レイヤー
		CollisionLayer mask = std::numeric_limits<CollisionLayer>::max(); // 衝突対象として許可するレイヤー集合
		uint64_t userData = 0;                                            // 利用者が関連付けた識別値
		bool continuous = false;                                          // 球の連続判定を行うか
		bool isTrigger = false;                                           // Triggerとして通知だけを行うか
		bool enabled = true;                                              // 衝突判定へ参加するか
		CollisionCallback callback{};                                     // 衝突イベントの通知先
	};

	/// <summary>
	/// 2つのCollider番号を順序に依存せず表す組。
	/// </summary>
	struct Pair {
		uint64_t first = 0;  // 小さい方のCollider番号
		uint64_t second = 0; // 大きい方のCollider番号

		/// <summary>
		/// 同じColliderの組か比較する。
		/// </summary>
		bool operator==(const Pair &) const noexcept = default;
	};

	/// <summary>
	/// Pairをunordered_setで使用するためのハッシュ生成器。
	/// </summary>
	struct PairHash {
		/// <summary>
		/// Collider番号の組からハッシュ値を生成する。
		/// </summary>
		size_t operator()(const Pair &pair) const noexcept;
	};

	/// <summary>
	/// 2つのCollider番号を常に同じ順序へ正規化する。
	/// </summary>
	static Pair MakePair(uint64_t first, uint64_t second);

	/// <summary>
	/// 2つの内部Colliderが現在交差しているか判定する。
	/// </summary>
	bool Intersects(const Collider &first, const Collider &second) const;

	/// <summary>
	/// 指定した組の両Colliderへ衝突イベントを通知する。
	/// </summary>
	void Dispatch(const Pair &pair, CollisionEventType type);

	/// <summary>
	/// 
	/// ハンドルに対応するColliderを取得し、無効なら例外を送出する。</summary>
	Collider &Require(ColliderHandle handle);

	std::unordered_map<uint64_t, Collider> colliders_; // ハンドル番号から登録済みColliderを引く表
	std::unordered_set<Pair, PairHash> activePairs_;   // 前回更新時にも接触していたColliderの組
	uint64_t nextHandle_ = 1;                          // 次回登録時に発行するCollider番号
};
