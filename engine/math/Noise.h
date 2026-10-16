#pragma once

#include "engine/math/MathUtility.h"

#include <cmath>
#include <cstdint>

namespace Noise {
/// <summary>
/// 整数値を十分に拡散した再現可能な32ビット値へ変換する。
/// </summary>
inline uint32_t Hash(uint32_t value) {
	value ^= value >> 16;
	value *= 0x7feb352du;
	value ^= value >> 15;
	value *= 0x846ca68bu;
	return value ^ (value >> 16);
}

/// <summary>
/// 整数のハッシュ値を-1から1の浮動小数点数へ変換する。
/// </summary>
inline float HashToSignedFloat(uint32_t value) {
	return static_cast<float>(Hash(value) & 0x00ffffffu) / 8388607.5f - 1.0f;
}

/// <summary>
/// 位置とシードから再現可能な-1から1の連続ノイズ値を生成する。
/// </summary>
inline float Value1D(float position, uint32_t seed = 0) {
	const int32_t left = static_cast<int32_t>(std::floor(position));
	const int32_t right = left + 1;
	const float fraction = position - static_cast<float>(left);
	const float smooth = fraction * fraction * (3.0f - 2.0f * fraction);
	const float first = HashToSignedFloat(static_cast<uint32_t>(left) ^ seed);
	const float second = HashToSignedFloat(static_cast<uint32_t>(right) ^ seed);
	return Lerp(first, second, smooth);
}

/// <summary>
/// 異なるシードの一次元ノイズから三次元の連続ノイズ値を生成する。
/// </summary>
inline Vector3 Value3D(float position, uint32_t seed = 0) {
	return {Value1D(position, seed), Value1D(position, seed + 0x9e3779b9u), Value1D(position, seed + 0x85ebca6bu)};
}
} // namespace Noise
