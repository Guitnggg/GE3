#pragma once

#include <cstdint>

/// <summary>ゲームプレイ中に調整できるパラメータ一式。</summary>
struct GameParameters {
	float railSpeed = 8.0f;
	float aimSpeed = 520.0f;
	float enemySpawnInterval = 0.85f;
	float minimumSpawnInterval = 0.38f;
	float spawnAccelerationPerScore = 0.012f;
	float enemySpawnDistance = 62.0f;
	float enemyBaseRadius = 0.8f;
	float enemyRadiusStep = 0.14f;
	float enemyRotationSpeed = 1.5f;
	uint32_t startingLives = 3;
};

/// <summary>GameParametersを実行中に編集するデバッグUI。</summary>
class GameParameterEditor final {
public:
	/// <returns>ゲームの再スタートが要求された場合はtrue。</returns>
	bool Draw(GameParameters& parameters);
	bool IsPaused() const { return paused_; }
	void SetPaused(bool paused) { paused_ = paused; }

private:
	bool paused_ = false;
};
