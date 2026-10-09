#include "application/effects/MissileTrailEffect.h"

#include "engine/3D/camera/Camera.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

void MissileTrailEffect::Initialize(DirectXCommon *dxCommon,
                                    GPUParticlePipeline *pipeline,
                                    TextureManager *textureManager,
                                    uint32_t textureHandle) {
	if (initialized_) {
		throw std::logic_error("MissileTrailEffect is already initialized.");
	}
	particleSystem_.Initialize(dxCommon, pipeline, textureManager, textureHandle, 8192);
	GPUParticlePreset preset{};
	preset.acceleration = {0.0f, 1.2f, 0.0f};
	preset.drag = 1.4f;
	preset.startColor = {0.52f, 0.48f, 0.44f, 0.72f};
	preset.endColor = {0.08f, 0.08f, 0.09f, 0.0f};
	preset.startSize = {0.16f, 0.16f};
	preset.endSize = {0.72f, 0.72f};
	preset.minLifetime = 0.45f;
	preset.maxLifetime = 0.85f;
	preset.blendMode = GPUParticleBlendMode::Alpha;
	particleSystem_.SetPreset(preset);
	initialized_ = true;
}

void MissileTrailEffect::EmitTrails(const std::vector<Vector3> &missilePositions, float deltaTime) {
	if (!initialized_) {
		throw std::logic_error("MissileTrailEffect is not initialized.");
	}
	if (missilePositions.empty() || !std::isfinite(deltaTime) || deltaTime <= 0.0f) {
		return;
	}
	emissionAccumulator_ += kParticlesPerSecondPerMissile * deltaTime;
	const uint32_t count = std::min(static_cast<uint32_t>(emissionAccumulator_), 64u);
	if (count == 0) {
		return;
	}
	emissionAccumulator_ -= static_cast<float>(count);
	for (const Vector3 &position : missilePositions) {
		GPUParticleEmitData emit{};
		emit.position = position;
		emit.count = count;
		emit.minVelocity = {-0.55f, -0.55f, -0.55f};
		emit.maxVelocity = {0.55f, 0.55f, 0.55f};
		emit.positionSpread = {0.08f, 0.08f, 0.08f};
		emit.seed = 0x54524149u + emissionSequence_++ * 0x9e3779b9u;
		particleSystem_.Emit(emit);
	}
}

void MissileTrailEffect::Update(float deltaTime) {
	if (initialized_) {
		particleSystem_.Update(deltaTime);
	}
}

void MissileTrailEffect::Draw(const Camera &camera) {
	if (initialized_) {
		particleSystem_.Draw(camera);
	}
}

void MissileTrailEffect::Reset() {
	if (!initialized_) {
		return;
	}
	emissionAccumulator_ = 0.0f;
	emissionSequence_ = 0;
	particleSystem_.Reset();
}
