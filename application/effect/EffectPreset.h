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

} // namespace EffectPreset
