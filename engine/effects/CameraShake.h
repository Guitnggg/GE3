#pragma once

#include "engine/math/MathTypes.h"

#include <cstdint>

/// <summary>
/// 連続ノイズと減衰を使ってカメラ用の位置・回転オフセットを生成する。
/// カメラ本体へ依存しないため、任意のカメラ実装から利用できる。
/// </summary>
class CameraShake {
public:
	void Start(float duration, float positionAmplitude, float rotationAmplitude,
		float frequency = 20.0f, uint32_t seed = 0);
	void Update(float deltaTime);
	void Stop();

	bool IsPlaying() const { return isPlaying_; }
	Vector3 GetPositionOffset() const { return positionOffset_; }
	Vector3 GetRotationOffset() const { return rotationOffset_; }

private:
	float duration_ = 0.0f;
	float elapsed_ = 0.0f;
	float positionAmplitude_ = 0.0f;
	float rotationAmplitude_ = 0.0f;
	float frequency_ = 20.0f;
	uint32_t seed_ = 0;
	bool isPlaying_ = false;
	Vector3 positionOffset_{};
	Vector3 rotationOffset_{};
};
