#pragma once

#include "engine/math/MathUtility.h"

#include <algorithm>
#include <cmath>

namespace Interpolation {
inline float Damp(float current, float target, float damping, float deltaTime) {
	if (deltaTime <= 0.0f) {
		return current;
	}
	const float factor = 1.0f - std::exp(-std::max(0.0f, damping) * deltaTime);
	return Lerp(current, target, factor);
}

inline Vector2 Damp(const Vector2 &current, const Vector2 &target, float damping, float deltaTime) {
	if (deltaTime <= 0.0f) {
		return current;
	}
	return Lerp(current, target, 1.0f - std::exp(-std::max(0.0f, damping) * deltaTime));
}

inline Vector3 Damp(const Vector3 &current, const Vector3 &target, float damping, float deltaTime) {
	if (deltaTime <= 0.0f) {
		return current;
	}
	return Lerp(current, target, 1.0f - std::exp(-std::max(0.0f, damping) * deltaTime));
}

inline Vector4 Damp(const Vector4 &current, const Vector4 &target, float damping, float deltaTime) {
	if (deltaTime <= 0.0f) {
		return current;
	}
	return Lerp(current, target, 1.0f - std::exp(-std::max(0.0f, damping) * deltaTime));
}

inline float SmoothDamp(float current,
                        float target,
                        float &velocity,
                        float smoothTime,
                        float deltaTime,
                        float maxSpeed = 3.402823466e+38f) {
	if (deltaTime <= 0.0f) {
		return current;
	}
	smoothTime = std::max(0.0001f, smoothTime);
	const float omega = 2.0f / smoothTime;
	const float x = omega * deltaTime;
	const float decay = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
	const float originalTarget = target;
	float change = current - target;
	const float maxChange = std::max(0.0f, maxSpeed) * smoothTime;
	change = std::clamp(change, -maxChange, maxChange);
	target = current - change;
	const float temporary = (velocity + omega * change) * deltaTime;
	velocity = (velocity - omega * temporary) * decay;
	float output = target + (change + temporary) * decay;
	if ((originalTarget - current > 0.0f) == (output > originalTarget)) {
		output = originalTarget;
		velocity = 0.0f;
	}
	return output;
}

inline Vector3 SmoothDamp(const Vector3 &current,
                          const Vector3 &target,
                          Vector3 &velocity,
                          float smoothTime,
                          float deltaTime,
                          float maxSpeed = 3.402823466e+38f) {
	return {
	    SmoothDamp(current.x, target.x, velocity.x, smoothTime, deltaTime, maxSpeed),
	    SmoothDamp(current.y, target.y, velocity.y, smoothTime, deltaTime, maxSpeed),
	    SmoothDamp(current.z, target.z, velocity.z, smoothTime, deltaTime, maxSpeed),
	};
}

inline float Spring(float current, float target, float &velocity, float stiffness, float damping, float deltaTime) {
	if (deltaTime <= 0.0f) {
		return current;
	}
	const float acceleration = (target - current) * std::max(0.0f, stiffness) - velocity * std::max(0.0f, damping);
	velocity += acceleration * deltaTime;
	return current + velocity * deltaTime;
}

inline Vector3 Spring(
    const Vector3 &current, const Vector3 &target, Vector3 &velocity, float stiffness, float damping, float deltaTime) {
	return {
	    Spring(current.x, target.x, velocity.x, stiffness, damping, deltaTime),
	    Spring(current.y, target.y, velocity.y, stiffness, damping, deltaTime),
	    Spring(current.z, target.z, velocity.z, stiffness, damping, deltaTime),
	};
}
} // namespace Interpolation
