#include "engine/effects/particle/GPUParticleEmitter.h"

#include "engine/effects/particle/GPUParticleSystem.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

void GPUParticleEmitter::Initialize(GPUParticleSystem* system,
	const GPUParticleEmitData& emitTemplate, float intervalSeconds) {
	if (!system || !std::isfinite(intervalSeconds) || intervalSeconds <= 0.0f) {
		throw std::invalid_argument("GPUParticleEmitter requires a system and a positive interval.");
	}
	system_ = system;
	emitTemplate_ = emitTemplate;
	interval_ = intervalSeconds;
	Reset();
}

void GPUParticleEmitter::Update(float deltaTime, const Vector3& position) {
	if (!isActive_ || !std::isfinite(deltaTime) || deltaTime <= 0.0f) { return; }
	elapsed_ += deltaTime;
	while (elapsed_ >= interval_) {
		elapsed_ -= interval_;
		EmitOnce(position);
	}
}

void GPUParticleEmitter::EmitOnce(const Vector3& position) {
	if (!system_) { throw std::logic_error("GPUParticleEmitter is not initialized."); }
	auto request = emitTemplate_;
	request.position = position;
	request.seed += emissionSequence_++ * 0x9e3779b9u;
	system_->Emit(request);
}

void GPUParticleEmitter::Reset() {
	elapsed_ = 0.0f;
	emissionSequence_ = 0;
	isActive_ = false;
}

void GPUParticleEmitter::SetInterval(float intervalSeconds) {
	if (!std::isfinite(intervalSeconds) || intervalSeconds <= 0.0f) {
		throw std::invalid_argument("GPUParticleEmitter interval must be positive.");
	}
	interval_ = intervalSeconds;
	elapsed_ = std::min(elapsed_, interval_);
}
