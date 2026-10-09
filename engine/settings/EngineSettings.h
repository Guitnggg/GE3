#pragma once

#include "engine/core/timing/FrameRateController.h"

#include <cstdint>
#include <filesystem>
#include <string>

class Time;

struct EngineSettings {
	FrameRateMode frameRateMode = FrameRateMode::VSync;
	double targetFPS = 60.0;
	float timeScale = 1.0f;
	float fixedDeltaTime = 1.0f / 60.0f;
	uint32_t maxFixedStepsPerFrame = 8;

	bool Load(const std::filesystem::path &path, std::string *errorMessage = nullptr);
	bool Save(const std::filesystem::path &path, std::string *errorMessage = nullptr) const;
	void Apply(FrameRateController &frameRateController, Time &time) const;
};
