#include "Time.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

void Time::Initialize() {
	if (initialized_) {
		throw std::logic_error("Time is already initialized.");
	}

	previousTime_ = Clock::now();
	deltaTime_ = 0.0f;
	unscaledDeltaTime_ = 0.0f;
	elapsedTime_ = 0.0;
	unscaledElapsedTime_ = 0.0;
	frameCount_ = 0;
	timeScale_ = kDefaultTimeScale;
	maxDeltaTime_ = kDefaultMaxDeltaTime;
	hasPreviousFrame_ = false;
	initialized_ = true;
}

void Time::Update() {
	if (!initialized_) {
		throw std::logic_error("Time is not initialized.");
	}

	const Clock::time_point currentTime = Clock::now();
	if (!hasPreviousFrame_) {
		previousTime_ = currentTime;
		hasPreviousFrame_ = true;
		deltaTime_ = 0.0f;
		unscaledDeltaTime_ = 0.0f;
		++frameCount_;
		return;
	}

	const std::chrono::duration<double> elapsed = currentTime - previousTime_;
	previousTime_ = currentTime;
	const double rawDeltaTime = std::max(0.0, elapsed.count());

	unscaledDeltaTime_ = static_cast<float>(
		std::min(rawDeltaTime, static_cast<double>(maxDeltaTime_)));
	deltaTime_ = unscaledDeltaTime_ * timeScale_;
	unscaledElapsedTime_ += rawDeltaTime;
	elapsedTime_ += static_cast<double>(deltaTime_);
	++frameCount_;
}

void Time::Finalize() {
	deltaTime_ = 0.0f;
	unscaledDeltaTime_ = 0.0f;
	elapsedTime_ = 0.0;
	unscaledElapsedTime_ = 0.0;
	frameCount_ = 0;
	timeScale_ = kDefaultTimeScale;
	maxDeltaTime_ = kDefaultMaxDeltaTime;
	hasPreviousFrame_ = false;
	initialized_ = false;
}

void Time::SetTimeScale(float timeScale) {
	if (!std::isfinite(timeScale) || timeScale < 0.0f) {
		throw std::invalid_argument("Time scale must be finite and non-negative.");
	}
	timeScale_ = timeScale;
}

void Time::SetMaxDeltaTime(float maxDeltaTime) {
	if (!std::isfinite(maxDeltaTime) || maxDeltaTime <= 0.0f) {
		throw std::invalid_argument("Maximum delta time must be finite and positive.");
	}
	maxDeltaTime_ = maxDeltaTime;
}
