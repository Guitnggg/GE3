#pragma once

#include "engine/core/timing/FrameRateController.h"

#include <cstdint>
#include <filesystem>
#include <string>

class Time;

struct EngineSettings {
	FrameRateMode frameRateMode = FrameRateMode::VSync; // 画面更新の同期方式
	double targetFPS = 60.0;                            // Limitedモードで目標とするFPS
	float timeScale = 1.0f;                             // ゲーム内時間へ掛ける倍率
	float fixedDeltaTime = 1.0f / 60.0f;                // 固定更新1回分の秒数
	uint32_t maxFixedStepsPerFrame = 8;                 // 1フレームで許可する固定更新回数

	/// <summary>
	/// JSONファイルからエンジン設定を読み込む。
	/// </summary>
	bool Load(const std::filesystem::path &path, std::string *errorMessage = nullptr);

	/// <summary>
	/// 現在のエンジン設定をJSONファイルへ保存する。
	/// </summary>
	bool Save(const std::filesystem::path &path, std::string *errorMessage = nullptr) const;

	/// <summary>
	/// 読み込んだ設定を時間管理とフレームレート管理へ反映する。
	/// </summary>
	void Apply(FrameRateController &frameRateController, Time &time) const;
};
