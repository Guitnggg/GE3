#include "engine/collision/Collision.h"

#include <algorithm>
#include <cmath>

namespace {
	// 平方根を使わず、2点間距離の二乗を倍精度で計算する
	double DistanceSquared(const Vector3& first, const Vector3& second) noexcept {
		const double x = static_cast<double>(first.x) - static_cast<double>(second.x);
		const double y = static_cast<double>(first.y) - static_cast<double>(second.y);
		const double z = static_cast<double>(first.z) - static_cast<double>(second.z);
		return x * x + y * y + z * z;
	}

	// NaNや無限大を含む座標を衝突計算へ渡さないため検証する
	bool IsFinite(const Vector3& value) noexcept {
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
	}
}

bool Collision::IsValid(const SphereCollider& sphere) noexcept {
	// 中心が有限値で、半径が負でなければ有効とする
	return IsFinite(sphere.center) && std::isfinite(sphere.radius) && sphere.radius >= 0.0f;
}

bool Collision::IsValid(const AabbCollider& aabb) noexcept {
	// 全成分が有限値で、各軸の最小値が最大値以下か確認する
	return IsFinite(aabb.min) && IsFinite(aabb.max) &&
		aabb.min.x <= aabb.max.x && aabb.min.y <= aabb.max.y && aabb.min.z <= aabb.max.z;
}

bool Collision::Intersects(const SphereCollider& first, const SphereCollider& second) noexcept {
	if (!IsValid(first) || !IsValid(second)) { return false; }
	// 中心間距離が半径の合計以下なら、接触を含めて交差している
	const double combinedRadius = static_cast<double>(first.radius) + static_cast<double>(second.radius);
	return DistanceSquared(first.center, second.center) <= combinedRadius * combinedRadius;
}

bool Collision::Intersects(const AabbCollider& first, const AabbCollider& second) noexcept {
	if (!IsValid(first) || !IsValid(second)) { return false; }
	// 3軸すべてで投影区間が重なっているか確認する
	return first.min.x <= second.max.x && first.max.x >= second.min.x &&
		first.min.y <= second.max.y && first.max.y >= second.min.y &&
		first.min.z <= second.max.z && first.max.z >= second.min.z;
}

bool Collision::Intersects(const SphereCollider& sphere, const AabbCollider& aabb) noexcept {
	if (!IsValid(sphere) || !IsValid(aabb)) { return false; }
	// 球の中心をAABB内へクランプし、最も近い点を求める
	const Vector3 closestPoint{
		std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
		std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
		std::clamp(sphere.center.z, aabb.min.z, aabb.max.z),
	};
	// 最近接点までの距離が半径以下なら交差している
	const double radius = sphere.radius;
	return DistanceSquared(sphere.center, closestPoint) <= radius * radius;
}

bool Collision::Contains(const AabbCollider& aabb, const Vector3& point) noexcept {
	if (!IsValid(aabb) || !IsFinite(point)) { return false; }
	// 各軸が閉区間内にあるため、境界上の点も内包とみなす
	return point.x >= aabb.min.x && point.x <= aabb.max.x &&
		point.y >= aabb.min.y && point.y <= aabb.max.y &&
		point.z >= aabb.min.z && point.z <= aabb.max.z;
}
