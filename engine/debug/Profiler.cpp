#include "engine/debug/Profiler.h"

#include <algorithm>

Profiler &Profiler::Get() {
	static Profiler instance;
	return instance;
}

void Profiler::BeginFrame() {
	currentRecords_.clear();
}

void Profiler::EndFrame() {
	lastFrameRecords_.clear();
	lastFrameRecords_.reserve(currentRecords_.size());
	profiledMilliseconds_ = 0.0;
	for (const auto &[name, record] : currentRecords_) {
		(void)name;
		lastFrameRecords_.push_back(record);
		profiledMilliseconds_ += record.milliseconds;
	}
	std::sort(lastFrameRecords_.begin(), lastFrameRecords_.end(), [](const ProfileRecord &left, const ProfileRecord &right) {
		return left.milliseconds > right.milliseconds;
	});
}

void Profiler::Record(std::string_view name, double milliseconds) {
	ProfileRecord &record = currentRecords_[std::string(name)];
	record.name = name;
	record.milliseconds += milliseconds;
	++record.callCount;
}

ProfileScope::ProfileScope(std::string_view name) : name_(name), start_(Clock::now()) {}

ProfileScope::~ProfileScope() {
	const std::chrono::duration<double, std::milli> elapsed = Clock::now() - start_;
	Profiler::Get().Record(name_, elapsed.count());
}
