#pragma once

#include "engine/math/Easing.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

enum class TweenLoopMode {
	None,     // 終点へ到達したら停止する
	Restart,  // 終点へ到達したら始点から再開する
	PingPong, // 始点と終点の間を往復する
};

/// <summary>
/// 指定時間とイージングに沿って対応する数値型を始点から終点へ補間する。
/// </summary>
template <typename T> class Tween {
  public:
	/// <summary>
	/// 開始値、終了値、再生時間、補間方式を設定してTweenを開始する。
	/// </summary>
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

	/// <summary>
	/// 経過時間を進め、現在値とループ状態を更新する。
	/// </summary>
	/// <param name="deltaTime">今回進める時間</param>
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

	/// <summary>
	/// 現在値を維持したままTweenの再生を停止する。
	/// </summary>
	void Stop() {
		isPlaying_ = false;
	}

	/// <summary>
	/// 開始値へ戻し、未再生状態にする。
	/// </summary>
	void Reset() {
		value_ = start_;
		elapsed_ = 0.0f;
		forward_ = true;
		isPlaying_ = false;
		isComplete_ = false;
	}

	/// <summary>
	/// 現在補間されている値を取得する。
	/// </summary>
	const T &GetValue() const {
		return value_;
	}

	/// <summary>
	/// Tweenが現在進行中か取得する。
	/// </summary>
	bool IsPlaying() const {
		return isPlaying_;
	}

	/// <summary>
	/// ループしないTweenが終点へ到達したか取得する。
	/// </summary>
	bool IsComplete() const {
		return isComplete_;
	}

	/// <summary>
	/// 現在の進行位置を0から1の範囲で取得する。
	/// </summary>
	float GetNormalizedTime() const {
		if (duration_ <= 0.0f) {
			return 0.0f;
		}
		const float time = std::clamp(elapsed_ / duration_, 0.0f, 1.0f);
		return forward_ ? time : 1.0f - time;
	}

  private:
	T start_{};                                    // 補間を開始する値
	T end_{};                                      // 補間の終了値
	T value_{};                                    // 現在の補間結果
	float duration_ = 0.0f;                        // 始点から終点までにかける秒数
	float elapsed_ = 0.0f;                         // 現在方向へ進んだ経過秒数
	EasingType easing_ = EasingType::Linear;       // 補間係数へ適用するイージング方式
	TweenLoopMode loopMode_ = TweenLoopMode::None; // 終点到達後の繰り返し方式
	bool forward_ = true;                          // 始点から終点へ進行中か
	bool isPlaying_ = false;                       // Updateで時間を進める状態か
	bool isComplete_ = false;                      // ループせず終点へ到達したか
};
