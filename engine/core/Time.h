#pragma once

#include <chrono>
#include <cstdint>

/// <summary>
/// 描画FPSから独立したフレーム時間とゲーム内時間を管理するクラス。
/// </summary>
class Time {
public:
	static constexpr float kDefaultTimeScale = 1.0f;
	static constexpr float kDefaultMaxDeltaTime = 0.1f;

	Time() = default;
	Time(const Time&) = delete;
	Time& operator=(const Time&) = delete;

	/// <summary>
	/// 初期化処理。Timeクラスの内部状態を初期化する。
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレームの更新処理。経過時間を計算し、フレームカウントを更新する。
	/// </summary>
	void Update();

	/// <summary>
	/// 終了処理。Timeクラスのリソースを解放する。
	/// </summary>
	void Finalize();

	/// <summary>
	/// 前フレームからの経過時間（秒）を返す。ゲーム内時間の倍率が適用される。
	/// </summary>
	/// <returns>経過時間（秒）</returns>
	float GetDeltaTime() const { return deltaTime_; }

	/// <summary>
	/// 前フレームからの経過時間（秒）を返す。ゲーム内時間の倍率は適用されない。
	/// </summary>
	/// <returns>経過時間（秒）</returns>
	float GetUnscaledDeltaTime() const { return unscaledDeltaTime_; }

	/// <summary>
	/// ゲーム開始からの経過時間（秒）を返す。ゲーム内時間の倍率が適用される。
	/// </summary>
	/// <returns>経過時間（秒）</returns>
	double GetElapsedTime() const { return elapsedTime_; }

	/// <summary>
	/// ゲーム開始からの経過時間（秒）を返す。ゲーム内時間の倍率は適用されない。
	/// </summary>
	/// <returns>経過時間（秒）</returns>
	double GetUnscaledElapsedTime() const { return unscaledElapsedTime_; }

	/// <summary>
	/// ゲーム開始からのフレーム数を返す。
	/// </summary>
	/// <returns>フレーム数</returns>
	uint64_t GetFrameCount() const { return frameCount_; }

	/// <summary>
	/// ゲーム内時間の倍率を返す。0で停止、1で通常速度。
	/// </summary>
	/// <returns>ゲーム内時間の倍率</returns>
	float GetTimeScale() const { return timeScale_; }

	/// <summary>
	/// デバッグ停止などによる極端な時間飛びを抑える上限秒数を返す。
	/// </summary>
	/// <returns>上限秒数</returns>
	float GetMaxDeltaTime() const { return maxDeltaTime_; }

	/// <summary>
	/// Timeクラスが初期化されているかどうかを返す。
	/// </summary>
	/// <returns>初期化されているかどうか</returns>
	bool IsInitialized() const { return initialized_; }

	/// <summary>
	/// ゲーム内時間の倍率。0で停止、1で通常速度。
	/// </summary>
	/// <param name="timeScale">ゲーム内時間の倍率</param>
	void SetTimeScale(float timeScale);

	/// <summary>
	/// デバッグ停止などによる極端な時間飛びを抑える上限秒数。
	/// </summary>
	/// <param name="maxDeltaTime">上限秒数</param>
	void SetMaxDeltaTime(float maxDeltaTime);

private:
	/// 内部で使用する高精度クロックの型を定義
	using Clock = std::chrono::steady_clock;

	Clock::time_point previousTime_{};
	float deltaTime_ = 0.0f;
	float unscaledDeltaTime_ = 0.0f;
	double elapsedTime_ = 0.0;
	double unscaledElapsedTime_ = 0.0;
	uint64_t frameCount_ = 0;
	float timeScale_ = kDefaultTimeScale;
	float maxDeltaTime_ = kDefaultMaxDeltaTime;
	bool initialized_ = false;
	bool hasPreviousFrame_ = false;
};
