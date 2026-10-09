#pragma once

#include "engine/math/MathTypes.h"

#include <cstdint>

enum class GPUParticleBlendMode {
	Alpha,
	Additive,
};

/// <summary>
/// GPUパーティクルの運動と見た目を演出単位で定義する設定値。
/// </summary>
struct GPUParticlePreset {
	Vector3 acceleration{};
	float drag = 0.0f;
	Vector4 startColor{1.0f, 1.0f, 1.0f, 1.0f};
	Vector4 endColor{1.0f, 1.0f, 1.0f, 0.0f};
	Vector2 startSize{0.25f, 0.25f};
	Vector2 endSize{};
	float minLifetime = 0.5f;
	float maxLifetime = 1.0f;
	GPUParticleBlendMode blendMode = GPUParticleBlendMode::Alpha;
};

/// <summary>
/// 1回分の生成数と、GPUでランダム化する各パラメーター範囲。
/// </summary>
struct GPUParticleEmitData {
	Vector3 position{};
	uint32_t count = 1;
	Vector3 minVelocity{-1.0f, -1.0f, -1.0f};
	Vector3 maxVelocity{1.0f, 1.0f, 1.0f};
	Vector3 positionSpread{};
	uint32_t seed = 0;
};
