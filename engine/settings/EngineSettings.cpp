#include "engine/settings/EngineSettings.h"

#include "engine/core/timing/Time.h"
#include "engine/settings/SettingsStore.h"

namespace {
std::string ToString(FrameRateMode mode) {
	switch (mode) {
	case FrameRateMode::Limited: return "limited";
	case FrameRateMode::Unlimited: return "unlimited";
	default: return "vsync";
	}
}
FrameRateMode ToFrameRateMode(const std::string &value) {
	if (value == "limited") return FrameRateMode::Limited;
	if (value == "unlimited") return FrameRateMode::Unlimited;
	return FrameRateMode::VSync;
}
} // namespace

bool EngineSettings::Load(const std::filesystem::path &path, std::string *errorMessage) {
	SettingsStore store;
	if (!store.Load(path, errorMessage)) return false;
	frameRateMode = ToFrameRateMode(store.Get<std::string>("frameRate.mode", "vsync"));
	targetFPS = store.Get("frameRate.targetFPS", 60.0);
	timeScale = store.Get("time.scale", 1.0f);
	fixedDeltaTime = store.Get("time.fixedDeltaTime", 1.0f / 60.0f);
	maxFixedStepsPerFrame = store.Get("time.maxFixedStepsPerFrame", 8u);
	return true;
}

bool EngineSettings::Save(const std::filesystem::path &path, std::string *errorMessage) const {
	SettingsStore store;
	store.Set("frameRate.mode", ToString(frameRateMode));
	store.Set("frameRate.targetFPS", targetFPS);
	store.Set("time.scale", timeScale);
	store.Set("time.fixedDeltaTime", fixedDeltaTime);
	store.Set("time.maxFixedStepsPerFrame", maxFixedStepsPerFrame);
	return store.Save(path, errorMessage);
}

void EngineSettings::Apply(FrameRateController &frameRateController, Time &time) const {
	frameRateController.SetMode(frameRateMode);
	frameRateController.SetTargetFPS(targetFPS);
	time.SetTimeScale(timeScale);
	time.SetFixedDeltaTime(fixedDeltaTime);
	time.SetMaxFixedStepsPerFrame(maxFixedStepsPerFrame);
}
