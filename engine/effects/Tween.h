#pragma once

#include "engine/math/Easing.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

enum class TweenLoopMode {
	None,
	Restart,
	PingPong,
};

/// <summary>
/// 時間経過に合わせて値を補間する汎用Tween。
/// float / Vector2 / Vector3 / Vector4で使用できる。
/// </summary>
template <typename T> class Tween {
  public:
	void Start(const T &start,
	           const T &end,
	           float duration,
	           EasingType easing = EasingType::Linear,
	           TweenLoopMode loopMode = TweenLoopMode::None) {
		if (!std::isfinite(duration) || duration <= 0.0f) {
			throw std::invalid_argument("Tween duration must be finite and greater than zero.");
		}

		start_ = start;
		end_ = end;
		value_ = start;
		duration_ = duration;
		elapsed_ = 0.0f;
		easing_ = easing;
		loopMode_ = loopMode;
		forward_ = true;
		isPlaying_ = true;
		isComplete_ = false;
	}

	void Update(float deltaTime) {
		if (!isPlaying_ || !std::isfinite(deltaTime) || deltaTime <= 0.0f) {
			return;
		}

		elapsed_ += deltaTime;
		if (loopMode_ == TweenLoopMode::None && elapsed_ >= duration_) {
			elapsed_ = duration_;
			value_ = forward_ ? end_ : start_;
			isPlaying_ = false;
			isComplete_ = true;
			return;
		}

		if (elapsed_ >= duration_) {
			const auto completedLoops = static_cast<unsigned long long>(elapsed_ / duration_);
			elapsed_ = std::fmod(elapsed_, duration_);
			if (loopMode_ == TweenLoopMode::PingPong && completedLoops % 2ull != 0ull) {
				forward_ = !forward_;
			}
		}

		float normalizedTime = std::clamp(elapsed_ / duration_, 0.0f, 1.0f);
		if (!forward_) {
			normalizedTime = 1.0f - normalizedTime;
		}
		value_ = EaseLerp(start_, end_, normalizedTime, easing_);
	}

	void Stop() {
		isPlaying_ = false;
	}

	void Reset() {
		value_ = start_;
		elapsed_ = 0.0f;
		forward_ = true;
		isPlaying_ = false;
		isComplete_ = false;
	}

	const T &GetValue() const {
		return value_;
	}
	bool IsPlaying() const {
		return isPlaying_;
	}
	bool IsComplete() const {
		return isComplete_;
	}

	float GetNormalizedTime() const {
		if (duration_ <= 0.0f) {
			return 0.0f;
		}
		const float time = std::clamp(elapsed_ / duration_, 0.0f, 1.0f);
		return forward_ ? time : 1.0f - time;
	}

  private:
	T start_{};
	T end_{};
	T value_{};
	float duration_ = 0.0f;
	float elapsed_ = 0.0f;
	EasingType easing_ = EasingType::Linear;
	TweenLoopMode loopMode_ = TweenLoopMode::None;
	bool forward_ = true;
	bool isPlaying_ = false;
	bool isComplete_ = false;
};
