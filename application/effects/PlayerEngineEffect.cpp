#include "application/effects/PlayerEngineEffect.h"

#include "engine/3D/camera/Camera.h"

#include <algorithm>
#include <stdexcept>

void PlayerEngineEffect::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager,
	uint32_t textureHandle, const PlayerEngineEffectSettings& settings) {
	if (initialized_) { throw std::logic_error("PlayerEngineEffect is already initialized."); }
	particleSystem_.Initialize(dxCommon, textureManager, textureHandle, 4096);
	GPUParticleEmitData emit{};
	emit.seed = 0x454e474eu;
	emitter_.Initialize(&particleSystem_, emit, settings.interval);
	initialized_ = true;
	ApplySettings(settings);
}

void PlayerEngineEffect::Reset(const PlayerEngineEffectSettings& settings) {
	if (!initialized_) { return; }
	particleSystem_.Reset();
	emitter_.Reset();
	ApplySettings(settings);
}

void PlayerEngineEffect::Update(float deltaTime, const Vector3& playerPosition,
	const PlayerEngineEffectSettings& settings) {
	if (!initialized_) { throw std::logic_error("PlayerEngineEffect is not initialized."); }
	ApplySettings(settings);
	emitter_.Update(deltaTime, {playerPosition.x + settings.nozzleOffset.x,
		playerPosition.y + settings.nozzleOffset.y, playerPosition.z + settings.nozzleOffset.z});
	particleSystem_.Update(deltaTime);
}

void PlayerEngineEffect::Draw(const Camera& camera) {
	if (initialized_) { particleSystem_.Draw(camera); }
}

void PlayerEngineEffect::ApplySettings(const PlayerEngineEffectSettings& settings) {
	const float minLifetime = std::min(settings.minLifetime, settings.maxLifetime);
	const float maxLifetime = std::max(settings.minLifetime, settings.maxLifetime);
	const float minSpeed = std::min(settings.minSpeed, settings.maxSpeed);
	const float maxSpeed = std::max(settings.minSpeed, settings.maxSpeed);
	GPUParticlePreset preset{};
	preset.acceleration = {0.0f, 0.0f, settings.accelerationZ};
	preset.drag = settings.drag;
	preset.startColor = settings.startColor;
	preset.endColor = settings.endColor;
	preset.startSize = {settings.startSize, settings.startSize};
	preset.endSize = {settings.endSize, settings.endSize};
	preset.minLifetime = minLifetime;
	preset.maxLifetime = maxLifetime;
	preset.blendMode = GPUParticleBlendMode::Additive;
	particleSystem_.SetPreset(preset);

	GPUParticleEmitData emit{};
	emit.count = settings.count;
	emit.minVelocity = {-settings.velocitySpread, -settings.velocitySpread, -maxSpeed};
	emit.maxVelocity = {settings.velocitySpread, settings.velocitySpread, -minSpeed};
	emit.positionSpread = {settings.positionSpread, settings.positionSpread, settings.positionSpread};
	emit.seed = 0x454e474eu;
	emitter_.SetEmitTemplate(emit);
	emitter_.SetInterval(settings.interval);
	emitter_.SetActive(settings.enabled);
}
