#include "engine/effects/CameraShake.h"

#include "engine/math/Easing.h"
#include "engine/math/Noise.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

void CameraShake::Start(float duration, float positionAmplitude, float rotationAmplitude,
	float frequency, uint32_t seed) {
	if (!std::isfinite(duration) || duration <= 0.0f) {
		throw std::invalid_argument("CameraShake duration must be finite and greater than zero.");
	}
	if (!std::isfinite(positionAmplitude) || !std::isfinite(rotationAmplitude) ||
		!std::isfinite(frequency) || positionAmplitude < 0.0f ||
		rotationAmplitude < 0.0f || frequency < 0.0f) {
		throw std::invalid_argument("CameraShake amplitudes and frequency must be finite and non-negative.");
	}

	duration_ = duration;
	elapsed_ = 0.0f;
	positionAmplitude_ = positionAmplitude;
	rotationAmplitude_ = rotationAmplitude;
	frequency_ = frequency;
	seed_ = seed;
	positionOffset_ = {};
	rotationOffset_ = {};
	isPlaying_ = true;
}

void CameraShake::Update(float deltaTime) {
	if (!isPlaying_ || !std::isfinite(deltaTime) || deltaTime <= 0.0f) {
		return;
	}

	elapsed_ = std::min(elapsed_ + deltaTime, duration_);
	if (elapsed_ >= duration_) {
		Stop();
		return;
	}

	const float progress = std::clamp(elapsed_ / duration_, 0.0f, 1.0f);
	const float envelope = 1.0f - Easing::CubicOut(progress);
	const float samplePosition = elapsed_ * frequency_;
	const Vector3 positionNoise = Noise::Value3D(samplePosition, seed_);
	const Vector3 rotationNoise = Noise::Value3D(samplePosition, seed_ + 0x517cc1b7u);

	const float positionScale = positionAmplitude_ * envelope;
	const float rotationScale = rotationAmplitude_ * envelope;
	positionOffset_ = {
		positionNoise.x * positionScale,
		positionNoise.y * positionScale,
		positionNoise.z * positionScale,
	};
	rotationOffset_ = {
		rotationNoise.x * rotationScale,
		rotationNoise.y * rotationScale,
		rotationNoise.z * rotationScale,
	};
}

void CameraShake::Stop() {
	isPlaying_ = false;
	positionOffset_ = {};
	rotationOffset_ = {};
}
