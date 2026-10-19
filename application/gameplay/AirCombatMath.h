#pragma once

#include "engine/3D/camera/Camera.h"
#include "engine/core/WinApp.h"
#include "engine/math/MathTypes.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace AirCombatMath {
inline constexpr float kPi = std::numbers::pi_v<float>;
inline constexpr float kScreenWidth = static_cast<float>(WinApp::kClientWidth);
inline constexpr float kScreenHeight = static_cast<float>(WinApp::kClientHeight);

inline Vector3 Add(const Vector3 &a, const Vector3 &b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vector3 Subtract(const Vector3 &a, const Vector3 &b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vector3 Scale(const Vector3 &value, float scale) { return {value.x * scale, value.y * scale, value.z * scale}; }
inline float Dot(const Vector3 &a, const Vector3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float Length(const Vector3 &value) { return std::sqrt(Dot(value, value)); }

inline Vector3 Normalize(const Vector3 &value, const Vector3 &fallback = {0.0f, 0.0f, 1.0f}) {
	const float length = Length(value);
	return length > 0.0001f ? Scale(value, 1.0f / length) : fallback;
}

inline Vector3 Forward(float yaw, float pitch) {
	const float cosPitch = std::cos(pitch);
	return {std::sin(yaw) * cosPitch, std::sin(pitch), std::cos(yaw) * cosPitch};
}

inline float MoveAngleTowards(float current, float target, float maxDelta) {
	const float difference = std::remainder(target - current, kPi * 2.0f);
	return current + std::clamp(difference, -maxDelta, maxDelta);
}

inline bool WorldToScreen(const Vector3 &world, const Camera &camera, Vector2 &screen) {
	const Matrix4x4 &matrix = camera.GetViewProjectionMatrix();
	const float x = world.x * matrix.m[0][0] + world.y * matrix.m[1][0] + world.z * matrix.m[2][0] + matrix.m[3][0];
	const float y = world.x * matrix.m[0][1] + world.y * matrix.m[1][1] + world.z * matrix.m[2][1] + matrix.m[3][1];
	const float w = world.x * matrix.m[0][3] + world.y * matrix.m[1][3] + world.z * matrix.m[2][3] + matrix.m[3][3];
	if (w <= 0.01f) return false;
	const float ndcX = x / w;
	const float ndcY = y / w;
	screen = {(ndcX + 1.0f) * 0.5f * kScreenWidth, (1.0f - ndcY) * 0.5f * kScreenHeight};
	return ndcX >= -1.15f && ndcX <= 1.15f && ndcY >= -1.15f && ndcY <= 1.15f;
}
} // namespace AirCombatMath
