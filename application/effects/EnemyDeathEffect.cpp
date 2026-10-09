#include "application/effects/EnemyDeathEffect.h"

#include "engine/3D/camera/Camera.h"

#include <stdexcept>

void EnemyDeathEffect::Initialize(DirectXCommon *dxCommon,
                                  GPUParticlePipeline *pipeline,
                                  TextureManager *textureManager,
                                  uint32_t textureHandle) {
	if (initialized_) {
		throw std::logic_error("EnemyDeathEffect is already initialized.");
	}
	flashSystem_.Initialize(dxCommon, pipeline, textureManager, textureHandle, 512);
	sparkSystem_.Initialize(dxCommon, pipeline, textureManager, textureHandle, 4096);
	smokeSystem_.Initialize(dxCommon, pipeline, textureManager, textureHandle, 2048);

	GPUParticlePreset flash{};
	flash.drag = 6.0f;
	flash.startColor = {1.0f, 1.0f, 0.82f, 1.0f};
	flash.endColor = {1.0f, 0.25f, 0.02f, 0.0f};
	flash.startSize = {0.55f, 0.55f};
	flash.endSize = {1.65f, 1.65f};
	flash.minLifetime = 0.08f;
	flash.maxLifetime = 0.16f;
	flash.blendMode = GPUParticleBlendMode::Additive;
	flashSystem_.SetPreset(flash);

	GPUParticlePreset sparks{};
	sparks.acceleration = {0.0f, -7.0f, 0.0f};
	sparks.drag = 0.7f;
	sparks.startColor = {1.0f, 0.72f, 0.08f, 1.0f};
	sparks.endColor = {1.0f, 0.03f, 0.0f, 0.0f};
	sparks.startSize = {0.14f, 0.14f};
	sparks.endSize = {0.015f, 0.015f};
	sparks.minLifetime = 0.30f;
	sparks.maxLifetime = 0.85f;
	sparks.blendMode = GPUParticleBlendMode::Additive;
	sparkSystem_.SetPreset(sparks);

	GPUParticlePreset smoke{};
	smoke.acceleration = {0.0f, 1.6f, 0.0f};
	smoke.drag = 2.2f;
	smoke.startColor = {0.30f, 0.24f, 0.20f, 0.62f};
	smoke.endColor = {0.07f, 0.07f, 0.08f, 0.0f};
	smoke.startSize = {0.38f, 0.38f};
	smoke.endSize = {1.35f, 1.35f};
	smoke.minLifetime = 0.55f;
	smoke.maxLifetime = 1.15f;
	smoke.blendMode = GPUParticleBlendMode::Alpha;
	smokeSystem_.SetPreset(smoke);
	initialized_ = true;
}

void EnemyDeathEffect::Emit(const Vector3 &position) {
	if (!initialized_) {
		throw std::logic_error("EnemyDeathEffect is not initialized.");
	}
	const uint32_t seed = 0x424f4f4du + emissionSequence_++ * 0x9e3779b9u;
	GPUParticleEmitData flash{};
	flash.position = position;
	flash.count = 28;
	flash.minVelocity = {-2.0f, -2.0f, -2.0f};
	flash.maxVelocity = {2.0f, 2.0f, 2.0f};
	flash.positionSpread = {0.12f, 0.12f, 0.12f};
	flash.seed = seed;
	flashSystem_.Emit(flash);

	GPUParticleEmitData sparks{};
	sparks.position = position;
	sparks.count = 220;
	sparks.minVelocity = {-14.0f, -14.0f, -14.0f};
	sparks.maxVelocity = {14.0f, 14.0f, 14.0f};
	sparks.positionSpread = {0.18f, 0.18f, 0.18f};
	sparks.seed = seed ^ 0x85ebca6bu;
	sparkSystem_.Emit(sparks);

	GPUParticleEmitData smoke{};
	smoke.position = position;
	smoke.count = 72;
	smoke.minVelocity = {-3.2f, 0.3f, -3.2f};
	smoke.maxVelocity = {3.2f, 4.2f, 3.2f};
	smoke.positionSpread = {0.28f, 0.28f, 0.28f};
	smoke.seed = seed ^ 0xc2b2ae35u;
	smokeSystem_.Emit(smoke);
}

void EnemyDeathEffect::Update(float deltaTime) {
	if (!initialized_) {
		return;
	}
	flashSystem_.Update(deltaTime);
	sparkSystem_.Update(deltaTime);
	smokeSystem_.Update(deltaTime);
}

void EnemyDeathEffect::Draw(const Camera &camera) {
	if (!initialized_) {
		return;
	}
	// 半透明の煙を先に描き、白熱コアと火花を加算して明るさを重ねる。
	smokeSystem_.Draw(camera);
	flashSystem_.Draw(camera);
	sparkSystem_.Draw(camera);
}

void EnemyDeathEffect::Reset() {
	if (!initialized_) {
		return;
	}
	emissionSequence_ = 0;
	flashSystem_.Reset();
	sparkSystem_.Reset();
	smokeSystem_.Reset();
}
