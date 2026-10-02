#pragma once

#include "engine/collision/Collision.h"

#include <cstdint>
#include <functional>
#include <limits>
#include <unordered_map>
#include <unordered_set>

using CollisionLayer = uint32_t;

/// <summary>
/// CollisionWorldへ登録したColliderを識別するハンドル。
/// </summary>
struct ColliderHandle {
	uint64_t value = 0;
	explicit operator bool() const noexcept { return value != 0; }
	bool operator==(const ColliderHandle&) const noexcept = default;
};

enum class CollisionEventType { Enter, Stay, Exit };

/// <summary>
/// Collider自身から見た衝突イベント。
/// </summary>
struct CollisionEvent {
	CollisionEventType type = CollisionEventType::Enter;
	ColliderHandle self{};
	ColliderHandle other{};
	CollisionLayer otherLayer = 0;
	uint64_t otherUserData = 0;
};

using CollisionCallback = std::function<void(const CollisionEvent&)>;

/// <summary>
/// Sphere Colliderの登録情報。
/// </summary>
struct SphereColliderDesc {
	SphereCollider shape{};
	CollisionLayer layer = 1;
	CollisionLayer mask = std::numeric_limits<CollisionLayer>::max();
	uint64_t userData = 0;
	bool continuous = false;
	CollisionCallback callback{};
};

/// <summary>
/// AABB Colliderの登録情報。
/// </summary>
struct AabbColliderDesc {
	AabbCollider shape{};
	CollisionLayer layer = 1;
	CollisionLayer mask = std::numeric_limits<CollisionLayer>::max();
	uint64_t userData = 0;
	CollisionCallback callback{};
};

struct RaycastHit {
	ColliderHandle collider{};
	Vector3 point{};
	Vector3 normal{};
	float distance = 0.0f;
	CollisionLayer layer = 0;
	uint64_t userData = 0;
};

/// <summary>
/// Colliderの登録、レイヤーフィルタ、衝突状態、Raycastを一括管理する。
/// Updateは全Colliderの位置更新後、破棄処理前に1回呼び出す。
/// </summary>
class CollisionWorld final {
public:
	ColliderHandle RegisterSphere(const SphereColliderDesc& desc);
	ColliderHandle RegisterAabb(const AabbColliderDesc& desc);
	void Unregister(ColliderHandle handle);
	void Clear();

	void SetSphere(ColliderHandle handle, const SphereCollider& sphere);
	void SetAabb(ColliderHandle handle, const AabbCollider& aabb);
	void SetEnabled(ColliderHandle handle, bool enabled);
	bool IsRegistered(ColliderHandle handle) const;

	/// <summary>
	/// 現在の形状からEnter/Stay/Exitを計算し、登録されたCallbackへ通知する。
	/// </summary>
	void Update();

	/// <summary>
	/// 指定レイヤーに属するColliderのうち、最も近い命中を返す。
	/// </summary>
	bool Raycast(const Vector3& origin, const Vector3& direction, float maxDistance,
		CollisionLayer layerMask, RaycastHit& hit) const;

	uint32_t GetColliderCount() const { return static_cast<uint32_t>(colliders_.size()); }

private:
	enum class ShapeType { Sphere, Aabb };
	struct Collider {
		ShapeType type = ShapeType::Sphere;
		SphereCollider sphere{};
		SphereCollider previousSphere{};
		AabbCollider aabb{};
		CollisionLayer layer = 1;
		CollisionLayer mask = std::numeric_limits<CollisionLayer>::max();
		uint64_t userData = 0;
		bool continuous = false;
		bool enabled = true;
		CollisionCallback callback{};
	};
	struct Pair {
		uint64_t first = 0;
		uint64_t second = 0;
		bool operator==(const Pair&) const noexcept = default;
	};
	struct PairHash {
		size_t operator()(const Pair& pair) const noexcept;
	};

	static Pair MakePair(uint64_t first, uint64_t second);
	bool Intersects(const Collider& first, const Collider& second) const;
	void Dispatch(const Pair& pair, CollisionEventType type);
	Collider& Require(ColliderHandle handle);

	std::unordered_map<uint64_t, Collider> colliders_;
	std::unordered_set<Pair, PairHash> activePairs_;
	uint64_t nextHandle_ = 1;
};
