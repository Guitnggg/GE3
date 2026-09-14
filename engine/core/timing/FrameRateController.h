#pragma once

#include <chrono>

/// <summary>
/// 描画の同期方法を表す列挙型。
/// </summary>
enum class FrameRateMode {
	VSync,		// 垂直同期を有効にする
	Limited,	// FPS上限をかける
	Unlimited,	// VSyncもFPS上限もかけない
};

/// <summary>
/// 描画の同期方法、FPS上限、実測フレームレートを管理するクラス。
/// </summary>
class FrameRateController {
public:
	/// <summary>
	/// 初期化処理。FrameRateControllerクラスの内部状態を初期化する。
	/// </summary>
	void Initialize();

	/// <summary>
	/// 1フレーム分の描画を開始する。前フレームの描画時間を計測し、FPS上限に応じて待機する。
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// 1フレーム分の描画を完了する。フレーム時間を計測し、実測FPSを更新する。
	/// </summary>
	void EndFrame();

	/// <summary>
	/// 終了処理。FrameRateControllerクラスのリソースを解放する。
	/// </summary>
	void Finalize();
	
	/// <summary>
	/// 描画の同期方法を設定する。
	/// </summary>
	/// <param name="mode">描画の同期方法</param>
	void SetMode(FrameRateMode mode);

	/// <summary>
	/// FPS上限を設定する。FPS上限はLimitedモードでのみ有効。
	/// </summary>
	/// <param name="targetFPS">目標FPS</param>
	void SetTargetFPS(double targetFPS);

public:
	/// <summary>
	/// 描画の同期方法を取得する。
	/// </summary>
	/// <returns>描画の同期方法</returns>
	FrameRateMode GetMode() const { return mode_; }

	/// <summary>
	/// FPS上限を取得する。
	/// </summary>
	/// <returns>FPS上限</returns>
	double GetTargetFPS() const { return targetFPS_; }

	/// <summary>
	/// 実測フレームレートを取得する。
	/// </summary>
	/// <returns>実測フレームレート</returns>
	double GetCurrentFPS() const { return currentFPS_; }

	/// <summary>
	/// 1フレームの描画時間をミリ秒単位で取得する。
	/// </summary>
	/// <returns>1フレームの描画時間（ミリ秒）</returns>
	double GetFrameTimeMilliseconds() const { return frameTimeMilliseconds_; }

	/// <summary>
	/// VSyncが有効かどうかを取得する。
	/// </summary>
	/// <returns>VSyncが有効な場合はtrue、それ以外の場合はfalse</returns>
	bool IsVSyncEnabled() const { return mode_ == FrameRateMode::VSync; }

	/// <summary>
	/// 初期化されているかどうかを取得する。
	/// </summary>
	/// <returns>初期化されている場合はtrue、それ以外の場合はfalse</returns>
	bool IsInitialized() const { return initialized_; }

private:
	// フレームレート計測用のクロック型を定義
	using Clock = std::chrono::steady_clock;

	FrameRateMode mode_ = FrameRateMode::VSync; // 現在使用する同期方式
	double targetFPS_ = 60.0;                   // Limitedモードの目標FPS
	double currentFPS_ = 0.0;                  // 平滑化した実測FPS
	double frameTimeMilliseconds_ = 0.0;       // 平滑化した1フレーム時間
	Clock::time_point frameStartTime_{};        // 現在のフレームを開始した時刻
	Clock::time_point previousFrameStartTime_{}; // FPS計測用の前フレーム開始時刻
	bool initialized_ = false;                  // Initialize済みかを示す
	bool hasPreviousFrame_ = false;             // FPS計算に使える前回時刻があるかを示す
};
