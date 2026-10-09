#pragma once

#include "engine/math/MathUtility.h"

#include <cmath>
#include <cstdint>

namespace Noise {
inline uint32_t Hash(uint32_t value) {
	value ^= value >> 16;
	value *= 0x7feb352du;
	value ^= value >> 15;
	value *= 0x846ca68bu;
	return value ^ (value >> 16);
}

inline float HashToSignedFloat(uint32_t value) {
	return static_cast<float>(Hash(value) & 0x00ffffffu) / 8388607.5f - 1.0f;
}

/// <summary>同じpositionとseedから常に同じ、連続した-1～1の値を返す。</summary>
inline float Value1D(float position, uint32_t seed = 0) {
	const int32_t left = static_cast<int32_t>(std::floor(position));
	const int32_t right = left + 1;
	const float fraction = position - static_cast<float>(left);
	const float smooth = fraction * fraction * (3.0f - 2.0f * fraction);
	const float first = HashToSignedFloat(static_cast<uint32_t>(left) ^ seed);
	const float second = HashToSignedFloat(static_cast<uint32_t>(right) ^ seed);
	return Lerp(first, second, smooth);
}

inline Vector3 Value3D(float position, uint32_t seed = 0) {
	return {Value1D(position, seed), Value1D(position, seed + 0x9e3779b9u), Value1D(position, seed + 0x85ebca6bu)};
}
} // namespace Noise
