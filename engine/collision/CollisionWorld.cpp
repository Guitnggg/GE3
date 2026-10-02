#include "engine/collision/CollisionWorld.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace {
float Dot(const Vector3& a, const Vector3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 AddScaled(const Vector3& origin, const Vector3& direction, float distance) {
	return {origin.x + direction.x * distance, origin.y + direction.y * distance,
		origin.z + direction.z * distance};
}

bool RaySphere(const Vector3& origin, const Vector3& direction, const SphereCollider& sphere,
	float maxDistance, float& distance, Vector3& normal) {
	const Vector3 offset{origin.x - sphere.center.x, origin.y - sphere.center.y, origin.z - sphere.center.z};
	const float b = Dot(offset, direction);
	const float c = Dot(offset, offset) - sphere.radius * sphere.radius;
	const float discriminant = b * b - c;
	if (discriminant < 0.0f) { return false; }
	const float root = std::sqrt(discriminant);
	float candidate = -b - root;
	if (candidate < 0.0f) { candidate = -b + root; }
	if (candidate < 0.0f || candidate > maxDistance) { return false; }
	distance = candidate;
	const Vector3 point = AddScaled(origin, direction, distance);
	normal = Normalize({point.x - sphere.center.x, point.y - sphere.center.y, point.z - sphere.center.z});
	return true;
}

bool RayAabb(const Vector3& origin, const Vector3& direction, const AabbCollider& aabb,
	float maxDistance, float& distance, Vector3& normal) {
	float nearDistance = 0.0f;
	float farDistance = maxDistance;
	Vector3 nearNormal{};
	const float origins[] = {origin.x, origin.y, origin.z};
	const float directions[] = {direction.x, direction.y, direction.z};
	const float minimums[] = {aabb.min.x, aabb.min.y, aabb.min.z};
	const float maximums[] = {aabb.max.x, aabb.max.y, aabb.max.z};
	for (int axis = 0; axis < 3; ++axis) {
		if (std::abs(directions[axis]) < 1.0e-6f) {
			if (origins[axis] < minimums[axis] || origins[axis] > maximums[axis]) { return false; }
			continue;
		}
		float first = (minimums[axis] - origins[axis]) / directions[axis];
		float second = (maximums[axis] - origins[axis]) / directions[axis];
		float sign = -1.0f;
		if (first > second) { std::swap(first, second); sign = 1.0f; }
		if (first > nearDistance) {
			nearDistance = first;
			nearNormal = {};
			if (axis == 0) { nearNormal.x = sign; }
			if (axis == 1) { nearNormal.y = sign; }
			if (axis == 2) { nearNormal.z = sign; }
		}
		farDistance = std::min(farDistance, second);
		if (nearDistance > farDistance) { return false; }
	}
	distance = nearDistance;
	normal = nearNormal;
	return distance <= maxDistance;
}
}

size_t CollisionWorld::PairHash::operator()(const Pair& pair) const noexcept {
	return std::hash<uint64_t>{}(pair.first) ^ (std::hash<uint64_t>{}(pair.second) << 1);
}

CollisionWorld::Pair CollisionWorld::MakePair(uint64_t first, uint64_t second) {
	return first < second ? Pair{first, second} : Pair{second, first};
}

ColliderHandle CollisionWorld::RegisterSphere(const SphereColliderDesc& desc) {
	if (!Collision::IsValid(desc.shape) || desc.layer == 0) {
		throw std::invalid_argument("Sphere collider requires a valid shape and non-zero layer.");
	}
	const ColliderHandle handle{nextHandle_++};
	Collider collider{};
	collider.type = ShapeType::Sphere;
	collider.sphere = desc.shape;
	collider.previousSphere = desc.shape;
	collider.layer = desc.layer;
	collider.mask = desc.mask;
	collider.userData = desc.userData;
	collider.continuous = desc.continuous;
	collider.callback = desc.callback;
	colliders_.emplace(handle.value, std::move(collider));
	return handle;
}

ColliderHandle CollisionWorld::RegisterAabb(const AabbColliderDesc& desc) {
	if (!Collision::IsValid(desc.shape) || desc.layer == 0) {
		throw std::invalid_argument("AABB collider requires a valid shape and non-zero layer.");
	}
	const ColliderHandle handle{nextHandle_++};
	Collider collider{};
	collider.type = ShapeType::Aabb;
	collider.aabb = desc.shape;
	collider.layer = desc.layer;
	collider.mask = desc.mask;
	collider.userData = desc.userData;
	collider.callback = desc.callback;
	colliders_.emplace(handle.value, std::move(collider));
	return handle;
}

void CollisionWorld::Unregister(ColliderHandle handle) {
	if (!handle || !colliders_.contains(handle.value)) { return; }
	std::vector<Pair> exits;
	for (const Pair& pair : activePairs_) {
		if (pair.first == handle.value || pair.second == handle.value) { exits.push_back(pair); }
	}
	for (const Pair& pair : exits) {
		const uint64_t remaining = pair.first == handle.value ? pair.second : pair.first;
		const auto remainingIt = colliders_.find(remaining);
		const auto removedIt = colliders_.find(handle.value);
		if (remainingIt != colliders_.end() && removedIt != colliders_.end() && remainingIt->second.callback) {
			remainingIt->second.callback({CollisionEventType::Exit, {remaining}, handle,
				removedIt->second.layer, removedIt->second.userData});
		}
		activePairs_.erase(pair);
	}
	colliders_.erase(handle.value);
}

void CollisionWorld::Clear() {
	activePairs_.clear();
	colliders_.clear();
}

CollisionWorld::Collider& CollisionWorld::Require(ColliderHandle handle) {
	const auto found = colliders_.find(handle.value);
	if (found == colliders_.end()) { throw std::invalid_argument("Collider handle is not registered."); }
	return found->second;
}

void CollisionWorld::SetSphere(ColliderHandle handle, const SphereCollider& sphere) {
	if (!Collision::IsValid(sphere)) { throw std::invalid_argument("Sphere collider is invalid."); }
	Collider& collider = Require(handle);
	if (collider.type != ShapeType::Sphere) { throw std::invalid_argument("Collider is not a sphere."); }
	collider.sphere = sphere;
}

void CollisionWorld::SetAabb(ColliderHandle handle, const AabbCollider& aabb) {
	if (!Collision::IsValid(aabb)) { throw std::invalid_argument("AABB collider is invalid."); }
	Collider& collider = Require(handle);
	if (collider.type != ShapeType::Aabb) { throw std::invalid_argument("Collider is not an AABB."); }
	collider.aabb = aabb;
}

void CollisionWorld::SetEnabled(ColliderHandle handle, bool enabled) { Require(handle).enabled = enabled; }

bool CollisionWorld::IsRegistered(ColliderHandle handle) const {
	return handle && colliders_.contains(handle.value);
}

bool CollisionWorld::Intersects(const Collider& first, const Collider& second) const {
	if (first.type == ShapeType::Sphere && second.type == ShapeType::Sphere) {
		const float combinedRadius = first.sphere.radius + second.sphere.radius;
		if (Collision::Intersects(first.sphere, second.sphere)) { return true; }
		if (first.continuous && Collision::IntersectsSegment(first.previousSphere.center,
			first.sphere.center, {second.sphere.center, combinedRadius})) { return true; }
		if (second.continuous && Collision::IntersectsSegment(second.previousSphere.center,
			second.sphere.center, {first.sphere.center, combinedRadius})) { return true; }
		return false;
	}
	if (first.type == ShapeType::Aabb && second.type == ShapeType::Aabb) {
		return Collision::Intersects(first.aabb, second.aabb);
	}
	const Collider& sphere = first.type == ShapeType::Sphere ? first : second;
	const Collider& aabb = first.type == ShapeType::Aabb ? first : second;
	return Collision::Intersects(sphere.sphere, aabb.aabb);
}

void CollisionWorld::Dispatch(const Pair& pair, CollisionEventType type) {
	const auto firstIt = colliders_.find(pair.first);
	const auto secondIt = colliders_.find(pair.second);
	if (firstIt == colliders_.end() || secondIt == colliders_.end()) { return; }
	const Collider first = firstIt->second;
	const Collider second = secondIt->second;
	if (first.callback) { first.callback({type, {pair.first}, {pair.second}, second.layer, second.userData}); }
	if (second.callback && colliders_.contains(pair.second)) {
		second.callback({type, {pair.second}, {pair.first}, first.layer, first.userData});
	}
}

void CollisionWorld::Update() {
	std::unordered_set<Pair, PairHash> currentPairs;
	std::vector<std::pair<Pair, CollisionEventType>> events;
	std::vector<uint64_t> handles;
	handles.reserve(colliders_.size());
	for (const auto& [handle, collider] : colliders_) {
		if (collider.enabled) { handles.push_back(handle); }
	}
	for (size_t firstIndex = 0; firstIndex < handles.size(); ++firstIndex) {
		for (size_t secondIndex = firstIndex + 1; secondIndex < handles.size(); ++secondIndex) {
			const Collider& first = colliders_.at(handles[firstIndex]);
			const Collider& second = colliders_.at(handles[secondIndex]);
			if ((first.mask & second.layer) == 0 || (second.mask & first.layer) == 0) { continue; }
			if (Intersects(first, second)) { currentPairs.insert(MakePair(handles[firstIndex], handles[secondIndex])); }
		}
	}
	for (const Pair& pair : currentPairs) {
		events.emplace_back(pair, activePairs_.contains(pair) ? CollisionEventType::Stay : CollisionEventType::Enter);
	}
	for (const Pair& pair : activePairs_) {
		if (!currentPairs.contains(pair)) { events.emplace_back(pair, CollisionEventType::Exit); }
	}
	// CallbackからColliderを解除しても走査中コンテナを壊さないよう、状態確定後に通知する。
	activePairs_ = std::move(currentPairs);
	for (const auto& [pair, type] : events) { Dispatch(pair, type); }
	for (auto& [handle, collider] : colliders_) {
		(void)handle;
		if (collider.type == ShapeType::Sphere) { collider.previousSphere = collider.sphere; }
	}
}

bool CollisionWorld::Raycast(const Vector3& origin, const Vector3& direction, float maxDistance,
	CollisionLayer layerMask, RaycastHit& hit) const {
	const float lengthSquared = Dot(direction, direction);
	if (!std::isfinite(maxDistance) || maxDistance < 0.0f || lengthSquared <= 1.0e-12f) {
		throw std::invalid_argument("Raycast requires a direction and finite non-negative distance.");
	}
	const float inverseLength = 1.0f / std::sqrt(lengthSquared);
	const Vector3 normalized{direction.x * inverseLength, direction.y * inverseLength, direction.z * inverseLength};
	bool found = false;
	float closest = maxDistance;
	for (const auto& [handle, collider] : colliders_) {
		if (!collider.enabled || (collider.layer & layerMask) == 0) { continue; }
		float distance = 0.0f;
		Vector3 normal{};
		const bool intersects = collider.type == ShapeType::Sphere
			? RaySphere(origin, normalized, collider.sphere, closest, distance, normal)
			: RayAabb(origin, normalized, collider.aabb, closest, distance, normal);
		if (!intersects || distance > closest) { continue; }
		found = true;
		closest = distance;
		hit = {{handle}, AddScaled(origin, normalized, distance), normal, distance, collider.layer, collider.userData};
	}
	return found;
}
