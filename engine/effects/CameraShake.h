#pragma once

#include "engine/math/MathTypes.h"

#include <cstdint>

/// <summary>
/// 連続ノイズと減衰を使ってカメラ用の位置・回転オフセットを生成する。
/// カメラ本体へ依存しないため、任意のカメラ実装から利用できる。
/// </summary>
class CameraShake {
  public:
	/// <summary>
	/// 揺れの長さ、強さ、周波数を指定して再生を開始する。
	/// </summary>
	void Start(
	    float duration, float positionAmplitude, float rotationAmplitude, float frequency = 20.0f, uint32_t seed = 0);
	/// <summary>
	/// 経過時間から減衰量と現在のノイズオフセットを計算する。
	/// </summary>
	void Update(float deltaTime);
	/// <summary>
	/// 再生を停止し、位置と回転のオフセットをゼロへ戻す。
	/// </summary>
	void Stop();

	/// <summary>
	/// 現在シェイクを再生しているか取得する。
	/// </summary>
	bool IsPlaying() const {
		return isPlaying_;
	}
	/// <summary>
	/// カメラ位置へ加算する現在のオフセットを取得する。
	/// </summary>
	Vector3 GetPositionOffset() const {
		return positionOffset_;
	}
	/// <summary>
	/// カメラ回転へ加算する現在のオフセットを取得する。
	/// </summary>
	Vector3 GetRotationOffset() const {
		return rotationOffset_;
	}

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
