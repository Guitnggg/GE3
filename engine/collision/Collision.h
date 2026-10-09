#pragma once

#include "engine/math/MathTypes.h"

/// <summary>
/// 中心位置と半径のみで構成する球形の衝突領域。
/// </summary>
struct SphereCollider {
	Vector3 center{};
	float radius = 0.5f;
};

/// <summary>
/// 最小点と最大点で構成する軸平行直方体の衝突領域。
/// </summary>
struct AabbCollider {
	Vector3 min{};
	Vector3 max{};
};

/// <summary>
/// 基本形状同士の交差判定を提供する名前空間。
/// 境界が触れている状態も衝突として扱う。
/// </summary>
namespace Collision {
/// <summary>
/// 球の半径が有効か確認する。
/// </summary>
bool IsValid(const SphereCollider &sphere) noexcept;

/// <summary>
/// AABBの最小値と最大値の順序が有効か確認する。
/// </summary>
bool IsValid(const AabbCollider &aabb) noexcept;

/// <summary>
/// 2つの球が交差または接触しているか判定する。
/// </summary>
bool Intersects(const SphereCollider &first, const SphereCollider &second) noexcept;

/// <summary>
/// 2つのAABBが交差または接触しているか判定する。
/// </summary>
bool Intersects(const AabbCollider &first, const AabbCollider &second) noexcept;

/// <summary>
/// 球とAABBが交差または接触しているか判定する。
/// </summary>
bool Intersects(const SphereCollider &sphere, const AabbCollider &aabb) noexcept;

/// <summary>
/// 線分が球と交差または接触しているか判定する。
/// </summary>
bool IntersectsSegment(const Vector3 &start, const Vector3 &end, const SphereCollider &sphere) noexcept;

/// <summary>
/// 点がAABBの内部または境界上にあるか判定する。
/// </summary>
bool Contains(const AabbCollider &aabb, const Vector3 &point) noexcept;
} // namespace Collision
