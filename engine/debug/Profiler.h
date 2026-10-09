#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct ProfileRecord {
	std::string name;          // 計測対象を識別する名前
	double milliseconds = 0.0; // フレーム内で消費した合計時間
	uint32_t callCount = 0;    // フレーム内で計測された回数
};

/// <summary>名前付きCPU処理区間をフレーム単位で集計するデバッグ用プロファイラー。</summary>
class Profiler final {
  public:
	static Profiler &Get();

	/// <summary>新しいフレームの計測を開始する。</summary>
	void BeginFrame();

	/// <summary>現在フレームの計測結果を表示用データとして確定する。</summary>
	void EndFrame();

	/// <summary>名前付き処理区間の実行時間を現在フレームへ加算する。</summary>
	void Record(std::string_view name, double milliseconds);

	/// <summary>直前に確定したフレームの計測結果を返す。</summary>
	[[nodiscard]] const std::vector<ProfileRecord> &GetLastFrameRecords() const {
		return lastFrameRecords_;
	}

	/// <summary>直前のフレームで計測対象が消費した合計時間を返す。</summary>
	[[nodiscard]] double GetProfiledMilliseconds() const {
		return profiledMilliseconds_;
	}

  private:
	Profiler() = default;
	std::unordered_map<std::string, ProfileRecord> currentRecords_; // 現在フレームの名前別計測値
	std::vector<ProfileRecord> lastFrameRecords_;                   // 表示用に確定した前フレームの計測値
	double profiledMilliseconds_ = 0.0;                             // 前フレームの計測区間合計時間
};

/// <summary>生成から破棄までの時間をProfilerへ記録するRAIIスコープ。</summary>
class ProfileScope final {
  public:
	explicit ProfileScope(std::string_view name);

	~ProfileScope();

	ProfileScope(const ProfileScope &) = delete;

	ProfileScope &operator=(const ProfileScope &) = delete;

  private:
	using Clock = std::chrono::steady_clock;
	std::string_view name_;   // Profilerへ登録する処理区間名
	Clock::time_point start_; // 区間計測を開始した時刻
};

#define ENGINE_PROFILE_CONCAT_IMPL(a, b) a##b
#define ENGINE_PROFILE_CONCAT(a, b) ENGINE_PROFILE_CONCAT_IMPL(a, b)
#ifdef _DEBUG
#define PROFILE_SCOPE(name) ProfileScope ENGINE_PROFILE_CONCAT(profileScope_, __LINE__)(name)
#else
#define PROFILE_SCOPE(name) ((void)0)
#endif
