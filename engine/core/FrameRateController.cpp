#include "FrameRateController.h"

#include <cmath>
#include <stdexcept>
#include <thread>

void FrameRateController::Initialize() {
	// 二重初期化によるフレーム計測状態の破棄を防ぐ
	if (initialized_) {
		throw std::logic_error("FrameRateController is already initialized.");
	}

	// 同期方式、目標値、計測結果を既定状態へ戻す
	mode_ = FrameRateMode::VSync;
	targetFPS_ = 60.0;
	currentFPS_ = 0.0;
	frameTimeMilliseconds_ = 0.0;
	hasPreviousFrame_ = false;
	initialized_ = true;
}

void FrameRateController::BeginFrame() {
	// フレーム開始時刻が必要なため、初期化前の呼び出しを拒否する
	if (!initialized_) {
		throw std::logic_error("FrameRateController is not initialized.");
	}

	// 最初のフレームは比較対象がないため、基準時刻だけ記録する
	frameStartTime_ = Clock::now();
	if (!hasPreviousFrame_) {
		previousFrameStartTime_ = frameStartTime_;
		hasPreviousFrame_ = true;
		return;
	}

	// 前回のフレーム開始から現在までの実経過時間を測る
	const std::chrono::duration<double> frameDuration = frameStartTime_ - previousFrameStartTime_;
	previousFrameStartTime_ = frameStartTime_;
	if (frameDuration.count() <= 0.0) {
		return;
	}

	// 表示値が細かく振動しないよう、指数移動平均でフレーム時間を平滑化する
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
	// Limited以外はPresentまたはGPU性能へ同期を任せ、CPU側では待機しない
	if (!initialized_) {
		throw std::logic_error("FrameRateController is not initialized.");
	}
	if (mode_ != FrameRateMode::Limited) {
		return;
	}

	// フレーム開始時刻から目標フレーム時間後を待機期限とする
	const std::chrono::duration<double> targetDuration(1.0 / targetFPS_);
	const Clock::time_point deadline = frameStartTime_ +
		std::chrono::duration_cast<Clock::duration>(targetDuration);
	constexpr auto kFineWaitDuration = std::chrono::microseconds(500);

	// 大部分はsleepでCPU負荷を抑え、最後だけyieldで待機精度を補う
	const Clock::time_point coarseWaitEnd = deadline - kFineWaitDuration;
	if (Clock::now() < coarseWaitEnd) {
		std::this_thread::sleep_until(coarseWaitEnd);
	}
	while (Clock::now() < deadline) {
		std::this_thread::yield();
	}
}

void FrameRateController::Finalize() {
	// 再初期化できるよう、同期設定と計測状態を既定値へ戻す
	mode_ = FrameRateMode::VSync;
	targetFPS_ = 60.0;
	currentFPS_ = 0.0;
	frameTimeMilliseconds_ = 0.0;
	hasPreviousFrame_ = false;
	initialized_ = false;
}

void FrameRateController::SetMode(FrameRateMode mode) {
	// 列挙型へ不正な整数値がキャストされた場合も受け付けない
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
	// 無限待機や過度なビジーループを避けるため、0より大きい1000FPS以下に制限する
	if (!initialized_) {
		throw std::logic_error("FrameRateController is not initialized.");
	}
	if (!std::isfinite(targetFPS) || targetFPS <= 0.0 || targetFPS > 1000.0) {
		throw std::invalid_argument("Target FPS must be finite, greater than zero, and at most 1000.");
	}
	targetFPS_ = targetFPS;
}
