#include "FrameRateController.h"

#include <cmath>
#include <stdexcept>
#include <thread>

void FrameRateController::Initialize() {
	if (initialized_) {
		throw std::logic_error("FrameRateController is already initialized.");
	}

	mode_ = FrameRateMode::VSync;
	targetFPS_ = 60.0;
	currentFPS_ = 0.0;
	frameTimeMilliseconds_ = 0.0;
	hasPreviousFrame_ = false;
	initialized_ = true;
}

void FrameRateController::BeginFrame() {
	if (!initialized_) {
		throw std::logic_error("FrameRateController is not initialized.");
	}

	frameStartTime_ = Clock::now();
	if (!hasPreviousFrame_) {
		previousFrameStartTime_ = frameStartTime_;
		hasPreviousFrame_ = true;
		return;
	}

	const std::chrono::duration<double> frameDuration = frameStartTime_ - previousFrameStartTime_;
	previousFrameStartTime_ = frameStartTime_;
	if (frameDuration.count() <= 0.0) {
		return;
	}

	const double measuredFrameTimeMilliseconds = frameDuration.count() * 1000.0;
	constexpr double kSmoothingFactor = 0.1;
	if (currentFPS_ == 0.0) {
		frameTimeMilliseconds_ = measuredFrameTimeMilliseconds;
	}
	else {
		frameTimeMilliseconds_ +=
			(measuredFrameTimeMilliseconds - frameTimeMilliseconds_) * kSmoothingFactor;
	}
	currentFPS_ = 1000.0 / frameTimeMilliseconds_;
}

void FrameRateController::EndFrame() {
	if (!initialized_) {
		throw std::logic_error("FrameRateController is not initialized.");
	}
	if (mode_ != FrameRateMode::Limited) {
		return;
	}

	const std::chrono::duration<double> targetDuration(1.0 / targetFPS_);
	const Clock::time_point deadline = frameStartTime_ +
		std::chrono::duration_cast<Clock::duration>(targetDuration);
	constexpr auto kFineWaitDuration = std::chrono::microseconds(500);

	const Clock::time_point coarseWaitEnd = deadline - kFineWaitDuration;
	if (Clock::now() < coarseWaitEnd) {
		std::this_thread::sleep_until(coarseWaitEnd);
	}
	while (Clock::now() < deadline) {
		std::this_thread::yield();
	}
}

void FrameRateController::Finalize() {
	mode_ = FrameRateMode::VSync;
	targetFPS_ = 60.0;
	currentFPS_ = 0.0;
	frameTimeMilliseconds_ = 0.0;
	hasPreviousFrame_ = false;
	initialized_ = false;
}

void FrameRateController::SetMode(FrameRateMode mode) {
	if (!initialized_) {
		throw std::logic_error("FrameRateController is not initialized.");
	}
	if (mode != FrameRateMode::VSync && mode != FrameRateMode::Limited &&
		mode != FrameRateMode::Unlimited) {
		throw std::invalid_argument("Invalid frame rate mode.");
	}
	mode_ = mode;
}

void FrameRateController::SetTargetFPS(double targetFPS) {
	if (!initialized_) {
		throw std::logic_error("FrameRateController is not initialized.");
	}
	if (!std::isfinite(targetFPS) || targetFPS <= 0.0 || targetFPS > 1000.0) {
		throw std::invalid_argument("Target FPS must be finite, greater than zero, and at most 1000.");
	}
	targetFPS_ = targetFPS;
}
