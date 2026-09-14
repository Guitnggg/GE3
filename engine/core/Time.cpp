#include "Time.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

void Time::Initialize() {
	// 二重初期化によって計測途中の状態が失われることを防ぐ
	if (initialized_) {
		throw std::logic_error("Time is already initialized.");
	}

	// 時刻、累積値、設定値を初期状態へ揃える
	previousTime_ = Clock::now();
	deltaTime_ = 0.0f;
	unscaledDeltaTime_ = 0.0f;
	elapsedTime_ = 0.0;
	unscaledElapsedTime_ = 0.0;
	fixedAccumulator_ = 0.0;
	fixedElapsedTime_ = 0.0;
	frameCount_ = 0;
	fixedFrameCount_ = 0;
	timeScale_ = kDefaultTimeScale;
	maxDeltaTime_ = kDefaultMaxDeltaTime;
	fixedDeltaTime_ = kDefaultFixedDeltaTime;
	maxFixedStepsPerFrame_ = kDefaultMaxFixedStepsPerFrame;
	fixedStepsThisFrame_ = 0;
	fixedTimeDroppedThisFrame_ = false;
	hasPreviousFrame_ = false;
	initialized_ = true;
}

void Time::Update() {
	// 初期化前は基準時刻を持たないため計測を拒否する
	if (!initialized_) {
		throw std::logic_error("Time is not initialized.");
	}
	fixedStepsThisFrame_ = 0;
	fixedTimeDroppedThisFrame_ = false;

	// 最初のフレームは比較対象がないため、経過時間0として基準時刻だけ記録する
	const Clock::time_point currentTime = Clock::now();
	if (!hasPreviousFrame_) {
		previousTime_ = currentTime;
		hasPreviousFrame_ = true;
		deltaTime_ = 0.0f;
		unscaledDeltaTime_ = 0.0f;
		++frameCount_;
		return;
	}

	// 高精度クロックから実際に経過した秒数を計算する
	const std::chrono::duration<double> elapsed = currentTime - previousTime_;
	previousTime_ = currentTime;
	const double rawDeltaTime = std::max(0.0, elapsed.count());

	// デバッグ停止などの大きな時間飛びはDeltaTimeだけ上限で抑える
	unscaledDeltaTime_ = static_cast<float>(
		std::min(rawDeltaTime, static_cast<double>(maxDeltaTime_)));
	deltaTime_ = unscaledDeltaTime_ * timeScale_;
	// 実時間とゲーム内時間を別々に累積する
	unscaledElapsedTime_ += rawDeltaTime;
	elapsedTime_ += static_cast<double>(deltaTime_);
	// TimeScale適用後の時間を蓄積し、固定更新側で一定量ずつ消費する
	fixedAccumulator_ += static_cast<double>(deltaTime_);
	++frameCount_;
}

bool Time::ConsumeFixedStep() {
	if (!initialized_) { throw std::logic_error("Time is not initialized."); }
	const double step = static_cast<double>(fixedDeltaTime_);
	if (fixedAccumulator_ < step) { return false; }

	// 上限を超える遅延は捨て、処理落ちがさらに固定更新を増やす悪循環を防ぐ
	if (fixedStepsThisFrame_ >= maxFixedStepsPerFrame_) {
		fixedAccumulator_ = std::fmod(fixedAccumulator_, step);
		fixedTimeDroppedThisFrame_ = true;
		return false;
	}

	fixedAccumulator_ -= step;
	fixedElapsedTime_ += step;
	++fixedFrameCount_;
	++fixedStepsThisFrame_;
	return true;
}

void Time::Finalize() {
	// 再初期化できるよう、計測値と設定値を既定状態へ戻す
	deltaTime_ = 0.0f;
	unscaledDeltaTime_ = 0.0f;
	elapsedTime_ = 0.0;
	unscaledElapsedTime_ = 0.0;
	fixedAccumulator_ = 0.0;
	fixedElapsedTime_ = 0.0;
	frameCount_ = 0;
	fixedFrameCount_ = 0;
	timeScale_ = kDefaultTimeScale;
	maxDeltaTime_ = kDefaultMaxDeltaTime;
	fixedDeltaTime_ = kDefaultFixedDeltaTime;
	maxFixedStepsPerFrame_ = kDefaultMaxFixedStepsPerFrame;
	fixedStepsThisFrame_ = 0;
	fixedTimeDroppedThisFrame_ = false;
	hasPreviousFrame_ = false;
	initialized_ = false;
}

void Time::SetTimeScale(float timeScale) {
	// NaN、無限大、負数は時間計算を壊すため拒否する。0は一時停止として許可する
	if (!std::isfinite(timeScale) || timeScale < 0.0f) {
		throw std::invalid_argument("Time scale must be finite and non-negative.");
	}
	timeScale_ = timeScale;
}

void Time::SetMaxDeltaTime(float maxDeltaTime) {
	// 上限は有限かつ正の秒数だけを受け付ける
	if (!std::isfinite(maxDeltaTime) || maxDeltaTime <= 0.0f) {
		throw std::invalid_argument("Maximum delta time must be finite and positive.");
	}
	maxDeltaTime_ = maxDeltaTime;
}

float Time::GetFixedInterpolationAlpha() const {
	if (fixedDeltaTime_ <= 0.0f) { return 0.0f; }
	return static_cast<float>(std::clamp(
		fixedAccumulator_ / static_cast<double>(fixedDeltaTime_), 0.0, 1.0));
}

void Time::SetFixedDeltaTime(float fixedDeltaTime) {
	if (!std::isfinite(fixedDeltaTime) || fixedDeltaTime <= 0.0f) {
		throw std::invalid_argument("Fixed delta time must be finite and positive.");
	}
	fixedDeltaTime_ = fixedDeltaTime;
	// 設定変更前の大きな持ち越し時間を、新しい更新間隔未満へ収める
	fixedAccumulator_ = std::fmod(fixedAccumulator_, static_cast<double>(fixedDeltaTime_));
}

void Time::SetMaxFixedStepsPerFrame(uint32_t maxSteps) {
	if (maxSteps == 0) { throw std::invalid_argument("Maximum fixed steps must be at least one."); }
	maxFixedStepsPerFrame_ = maxSteps;
}
