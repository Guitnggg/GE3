#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct ProfileRecord {
	std::string name;
	double milliseconds = 0.0;
	uint32_t callCount = 0;
};

/// <summary>名前付きCPU処理区間をフレーム単位で集計するデバッグ用プロファイラー。</summary>
class Profiler final {
  public:
	static Profiler &Get();

	void BeginFrame();
	void EndFrame();
	void Record(std::string_view name, double milliseconds);
	[[nodiscard]] const std::vector<ProfileRecord> &GetLastFrameRecords() const {
		return lastFrameRecords_;
	}
	[[nodiscard]] double GetProfiledMilliseconds() const {
		return profiledMilliseconds_;
	}

  private:
	Profiler() = default;
	std::unordered_map<std::string, ProfileRecord> currentRecords_;
	std::vector<ProfileRecord> lastFrameRecords_;
	double profiledMilliseconds_ = 0.0;
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
	std::string_view name_;
	Clock::time_point start_;
};

#define ENGINE_PROFILE_CONCAT_IMPL(a, b) a##b
#define ENGINE_PROFILE_CONCAT(a, b) ENGINE_PROFILE_CONCAT_IMPL(a, b)
#ifdef _DEBUG
#define PROFILE_SCOPE(name) ProfileScope ENGINE_PROFILE_CONCAT(profileScope_, __LINE__)(name)
#else
#define PROFILE_SCOPE(name) ((void)0)
#endif
