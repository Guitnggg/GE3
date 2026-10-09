#pragma once

#include "engine/math/MathTypes.h"

#include <algorithm>
#include <cmath>

/// <summary>
/// 3次元ベクトルを正規化する
/// </summary>
inline Vector3 Normalize(const Vector3 &v) {
	const float lengthSquared = v.x * v.x + v.y * v.y + v.z * v.z;
	constexpr float kLengthSquaredEpsilon = 1.0e-12f;
	if (lengthSquared <= kLengthSquaredEpsilon) {
		return {};
	}
	const float inverseLength = 1.0f / std::sqrt(lengthSquared);
	return {v.x * inverseLength, v.y * inverseLength, v.z * inverseLength};
}

/// <summary>2値を割合tで線形補間する。tは範囲外でもそのまま外挿する。</summary>
inline float Lerp(float start, float end, float t) {
	return start + (end - start) * t;
}

inline Vector2 Lerp(const Vector2 &start, const Vector2 &end, float t) {
	return {Lerp(start.x, end.x, t), Lerp(start.y, end.y, t)};
}

inline Vector3 Lerp(const Vector3 &start, const Vector3 &end, float t) {
	return {Lerp(start.x, end.x, t), Lerp(start.y, end.y, t), Lerp(start.z, end.z, t)};
}

inline Vector4 Lerp(const Vector4 &start, const Vector4 &end, float t) {
	return {Lerp(start.x, end.x, t), Lerp(start.y, end.y, t), Lerp(start.z, end.z, t), Lerp(start.s, end.s, t)};
}

/// <summary>valueがstartからendまでのどの割合にあるかを0～1で返す。</summary>
inline float InverseLerp(float start, float end, float value) {
	if (std::abs(end - start) <= 1.0e-6f) {
		return 0.0f;
	}
	return std::clamp((value - start) / (end - start), 0.0f, 1.0f);
}

/// <summary>入力範囲の値を出力範囲へ線形変換する。</summary>
inline float Remap(float inputStart, float inputEnd, float outputStart, float outputEnd, float value) {
	return Lerp(outputStart, outputEnd, InverseLerp(inputStart, inputEnd, value));
}

/// <summary>現在値をtargetへ最大maxDeltaだけ近づける。</summary>
inline float MoveTowards(float current, float target, float maxDelta) {
	if (maxDelta < 0.0f) {
		return current;
	}
	const float difference = target - current;
	if (std::abs(difference) <= maxDelta) {
		return target;
	}
	return current + std::copysign(maxDelta, difference);
}

/// <summary>valueを0以上length未満の周期へ折り返す。</summary>
inline float Repeat(float value, float length) {
	if (!std::isfinite(value) || !std::isfinite(length) || length <= 0.0f) {
		return 0.0f;
	}
	return value - std::floor(value / length) * length;
}

/// <summary>0からlengthまでを往復する値を返す。</summary>
inline float PingPong(float value, float length) {
	const float repeated = Repeat(value, length * 2.0f);
	return length - std::abs(repeated - length);
}

/// <summary>ラジアン角を-pi以上pi以下へ正規化する。</summary>
inline float NormalizeAngle(float radians) {
	constexpr float kPi = 3.14159265358979323846f;
	constexpr float kTau = kPi * 2.0f;
	float normalized = Repeat(radians + kPi, kTau) - kPi;
	return normalized == -kPi ? kPi : normalized;
}

/// <summary>currentからtargetまでの最短角度差をラジアンで返す。</summary>
inline float DeltaAngle(float current, float target) {
	return NormalizeAngle(target - current);
}

/// <summary>2つのラジアン角を最短方向へ補間する。</summary>
inline float LerpAngle(float start, float end, float t) {
	return start + DeltaAngle(start, end) * std::clamp(t, 0.0f, 1.0f);
}

/// <summary>現在角をtargetへ最大maxDeltaラジアンだけ最短方向へ近づける。</summary>
inline float MoveTowardsAngle(float current, float target, float maxDelta) {
	const float delta = DeltaAngle(current, target);
	if (std::abs(delta) <= maxDelta) {
		return target;
	}
	return current + std::copysign(std::max(0.0f, maxDelta), delta);
}
