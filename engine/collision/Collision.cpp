#include "engine/collision/Collision.h"

#include <algorithm>
#include <cmath>

namespace {
	double DistanceSquared(const Vector3& first, const Vector3& second) noexcept {
		const double x = static_cast<double>(first.x) - static_cast<double>(second.x);
		const double y = static_cast<double>(first.y) - static_cast<double>(second.y);
		const double z = static_cast<double>(first.z) - static_cast<double>(second.z);
		return x * x + y * y + z * z;
	}

	bool IsFinite(const Vector3& value) noexcept {
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
	}
}

bool Collision::IsValid(const SphereCollider& sphere) noexcept {
	return IsFinite(sphere.center) && std::isfinite(sphere.radius) && sphere.radius >= 0.0f;
}

bool Collision::IsValid(const AabbCollider& aabb) noexcept {
	return IsFinite(aabb.min) && IsFinite(aabb.max) &&
		aabb.min.x <= aabb.max.x && aabb.min.y <= aabb.max.y && aabb.min.z <= aabb.max.z;
}

bool Collision::Intersects(const SphereCollider& first, const SphereCollider& second) noexcept {
	if (!IsValid(first) || !IsValid(second)) { return false; }
	const double combinedRadius = static_cast<double>(first.radius) + static_cast<double>(second.radius);
	return DistanceSquared(first.center, second.center) <= combinedRadius * combinedRadius;
}

bool Collision::Intersects(const AabbCollider& first, const AabbCollider& second) noexcept {
	if (!IsValid(first) || !IsValid(second)) { return false; }
	return first.min.x <= second.max.x && first.max.x >= second.min.x &&
		first.min.y <= second.max.y && first.max.y >= second.min.y &&
		first.min.z <= second.max.z && first.max.z >= second.min.z;
}

bool Collision::Intersects(const SphereCollider& sphere, const AabbCollider& aabb) noexcept {
	if (!IsValid(sphere) || !IsValid(aabb)) { return false; }
	const Vector3 closestPoint{
		std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
		std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
		std::clamp(sphere.center.z, aabb.min.z, aabb.max.z),
	};
	const double radius = sphere.radius;
	return DistanceSquared(sphere.center, closestPoint) <= radius * radius;
}

bool Collision::Contains(const AabbCollider& aabb, const Vector3& point) noexcept {
	if (!IsValid(aabb) || !IsFinite(point)) { return false; }
	return point.x >= aabb.min.x && point.x <= aabb.max.x &&
		point.y >= aabb.min.y && point.y <= aabb.max.y &&
		point.z >= aabb.min.z && point.z <= aabb.max.z;
}
