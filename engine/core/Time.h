#pragma once

#include <chrono>
#include <cstdint>

/// <summary>
/// 描画FPSから独立したフレーム時間とゲーム内時間を管理するクラス。
/// </summary>
class Time {
public:
	static constexpr float kDefaultTimeScale = 1.0f;	 // ゲーム内時間の倍率。0で停止、1で通常速度
	static constexpr float kDefaultMaxDeltaTime = 0.1f;  // デバッグ停止などによる極端な時間飛びを抑える上限秒数
	static constexpr float kDefaultFixedDeltaTime = 1.0f / 60.0f; // 物理・固定ロジックを60Hzで更新する間隔
	static constexpr uint32_t kDefaultMaxFixedStepsPerFrame = 8; // 1描画フレームで追いつく固定更新回数の上限

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
	/// 固定更新1回分の時間を消費できるか判定する。
	/// trueが返る間、FrameworkがFixedUpdateを繰り返し呼び出す。
	/// </summary>
	/// <returns>固定更新を1回実行する必要がある場合はtrue</returns>
	bool ConsumeFixedStep();

	/// <summary>
	/// 終了処理。Timeクラスのリソースを解放する。
	/// </summary>
	void Finalize();

public:
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

	/// <summary>物理・固定ロジックへ渡す一定の経過秒数を取得する。</summary>
	float GetFixedDeltaTime() const { return fixedDeltaTime_; }

	/// <summary>固定更新開始からの累積ゲーム時間を取得する。</summary>
	double GetFixedElapsedTime() const { return fixedElapsedTime_; }

	/// <summary>ゲーム開始から実行した固定更新の総数を取得する。</summary>
	uint64_t GetFixedFrameCount() const { return fixedFrameCount_; }

	/// <summary>現在の描画フレーム内で実行した固定更新回数を取得する。</summary>
	uint32_t GetFixedStepsThisFrame() const { return fixedStepsThisFrame_; }

	/// <summary>1描画フレームで許可する固定更新の最大回数を取得する。</summary>
	uint32_t GetMaxFixedStepsPerFrame() const { return maxFixedStepsPerFrame_; }

	/// <summary>前回と次回の固定状態間を描画補間するための0～1の割合を取得する。</summary>
	float GetFixedInterpolationAlpha() const;

	/// <summary>更新上限を超えた固定更新時間を今フレームに破棄したか取得する。</summary>
	bool WasFixedTimeDroppedThisFrame() const { return fixedTimeDroppedThisFrame_; }

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

	/// <summary>物理・固定ロジックの更新間隔を秒単位で設定する。</summary>
	/// <param name="fixedDeltaTime">0より大きい有限の秒数</param>
	void SetFixedDeltaTime(float fixedDeltaTime);

	/// <summary>低FPS時に1描画フレームで追いつく固定更新回数の上限を設定する。</summary>
	/// <param name="maxSteps">1以上の更新回数</param>
	void SetMaxFixedStepsPerFrame(uint32_t maxSteps);

private:
	/// 内部で使用する高精度クロックの型を定義
	using Clock = std::chrono::steady_clock;

	Clock::time_point previousTime_{};            // 前回Updateを実行した時刻
	float deltaTime_ = 0.0f;                     // TimeScale適用後のフレーム経過秒
	float unscaledDeltaTime_ = 0.0f;             // TimeScale適用前のフレーム経過秒
	double elapsedTime_ = 0.0;                   // TimeScale適用後の累積時間
	double unscaledElapsedTime_ = 0.0;           // 実時間ベースの累積時間
	double fixedAccumulator_ = 0.0;              // 次の固定更新まで持ち越すゲーム内時間
	double fixedElapsedTime_ = 0.0;              // 固定更新で処理済みの累積ゲーム時間
	uint64_t frameCount_ = 0;                    // Updateが完了した回数
	uint64_t fixedFrameCount_ = 0;               // FixedUpdateを実行した総数
	float timeScale_ = kDefaultTimeScale;        // ゲーム内時間へ適用する倍率
	float maxDeltaTime_ = kDefaultMaxDeltaTime;  // 1フレームとして扱う最大秒数
	float fixedDeltaTime_ = kDefaultFixedDeltaTime; // 固定更新1回分の秒数
	uint32_t maxFixedStepsPerFrame_ = kDefaultMaxFixedStepsPerFrame; // 固定更新の追いつき上限
	uint32_t fixedStepsThisFrame_ = 0;            // 現在の描画フレームで消費した固定更新数
	bool initialized_ = false;                   // Initialize済みかを示す
	bool hasPreviousFrame_ = false;              // 時間差を取れる前回時刻があるかを示す
	bool fixedTimeDroppedThisFrame_ = false;      // 追いつき上限超過分を破棄したかを示す
};
