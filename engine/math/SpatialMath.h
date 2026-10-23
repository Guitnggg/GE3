#pragma once

#include "engine/math/MathTypes.h"

#include <cmath>

/// <summary>空間上のベクトル計算と画面投影に使用する汎用数学関数。</summary>
namespace SpatialMath {
inline Vector3 Add(const Vector3 &a, const Vector3 &b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vector3 Subtract(const Vector3 &a, const Vector3 &b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vector3 Scale(const Vector3 &value, float scale) { return {value.x * scale, value.y * scale, value.z * scale}; }
inline float Dot(const Vector3 &a, const Vector3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float Length(const Vector3 &value) { return std::sqrt(Dot(value, value)); }

inline Vector3 NormalizeOr(const Vector3 &value, const Vector3 &fallback = {0.0f, 0.0f, 1.0f}) {
	const float length = Length(value);
	return length > 0.0001f ? Scale(value, 1.0f / length) : fallback;
}

inline Vector3 DirectionFromYawPitch(float yaw, float pitch) {
	const float cosPitch = std::cos(pitch);
	return {std::sin(yaw) * cosPitch, std::sin(pitch), std::cos(yaw) * cosPitch};
}

inline bool WorldToScreen(const Vector3 &world,
	                      const Matrix4x4 &viewProjection,
	                      float viewportWidth,
	                      float viewportHeight,
	                      Vector2 &screen,
	                      float margin = 0.0f) {
	const float x = world.x * viewProjection.m[0][0] + world.y * viewProjection.m[1][0] +
	                world.z * viewProjection.m[2][0] + viewProjection.m[3][0];
	const float y = world.x * viewProjection.m[0][1] + world.y * viewProjection.m[1][1] +
	                world.z * viewProjection.m[2][1] + viewProjection.m[3][1];
	const float w = world.x * viewProjection.m[0][3] + world.y * viewProjection.m[1][3] +
	                world.z * viewProjection.m[2][3] + viewProjection.m[3][3];
	if (w <= 0.01f) return false;
	const float ndcX = x / w;
	const float ndcY = y / w;
	screen = {(ndcX + 1.0f) * 0.5f * viewportWidth, (1.0f - ndcY) * 0.5f * viewportHeight};
	return ndcX >= -1.0f - margin && ndcX <= 1.0f + margin &&
	       ndcY >= -1.0f - margin && ndcY <= 1.0f + margin;
}
} // namespace SpatialMath
