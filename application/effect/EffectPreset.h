#pragma once

#include "engine/effects/particle/GPUParticleSystem.h"

namespace EffectPreset {
/// <summary>
/// ミサイル航跡用のパーティクル設定を生成する
/// </summary>
inline GPUParticlePreset MissileTrail() {
	GPUParticlePreset preset{};
	preset.drag = 1.4f;
	preset.startColor = {1.0f, 0.65f, 0.18f, 0.85f};
	preset.endColor = {0.16f, 0.18f, 0.2f, 0.0f};
	preset.startSize = {1.3f, 1.3f};
	preset.endSize = {3.5f, 3.5f};
	preset.minLifetime = 0.35f;
	preset.maxLifetime = 0.8f;
	return preset;
}

/// <summary>爆発中心の高輝度な火球。</summary>
inline GPUParticlePreset ExplosionFire() {
	GPUParticlePreset preset{};
	preset.acceleration = {0.0f, 3.0f, 0.0f};
	preset.drag = 2.2f;
	preset.startColor = {1.0f, 0.82f, 0.2f, 1.0f};
	preset.endColor = {1.0f, 0.08f, 0.01f, 0.0f};
	preset.startSize = {3.2f, 3.2f};
	preset.endSize = {11.0f, 11.0f};
	preset.minLifetime = 0.3f;
	preset.maxLifetime = 0.75f;
	preset.blendMode = GPUParticleBlendMode::Additive;
	return preset;
}

/// <summary>爆発から高速で飛び散る火花と破片。</summary>
inline GPUParticlePreset ExplosionSparks() {
	GPUParticlePreset preset{};
	preset.acceleration = {0.0f, -18.0f, 0.0f};
	preset.drag = 0.35f;
	preset.startColor = {1.0f, 0.95f, 0.5f, 1.0f};
	preset.endColor = {1.0f, 0.15f, 0.01f, 0.0f};
	preset.startSize = {0.75f, 0.75f};
	preset.endSize = {0.08f, 0.08f};
	preset.minLifetime = 0.55f;
	preset.maxLifetime = 1.6f;
	preset.blendMode = GPUParticleBlendMode::Additive;
	return preset;
}

/// <summary>火球の後に膨張しながら残る煙。</summary>
inline GPUParticlePreset ExplosionSmoke() {
	GPUParticlePreset preset{};
	preset.acceleration = {0.0f, 5.5f, 0.0f};
	preset.drag = 0.9f;
	preset.startColor = {0.3f, 0.24f, 0.2f, 0.72f};
	preset.endColor = {0.05f, 0.06f, 0.07f, 0.0f};
	preset.startSize = {5.0f, 5.0f};
	preset.endSize = {19.0f, 19.0f};
	preset.minLifetime = 1.3f;
	preset.maxLifetime = 3.0f;
	return preset;
}

} // namespace EffectPreset
