#pragma once

#include "engine/effects/particle/GPUParticleSystem.h"

#include <cstdint>
#include <vector>

class Camera;
class DirectXCommon;
class GPUParticlePipeline;
class TextureManager;

/// <summary>
/// すべてのミサイルが共有する、軌道上の煙パーティクル。
/// </summary>
class MissileTrailEffect final {
  public:
	/// <summary>
	/// 全ミサイルで共有する煙パーティクルシステムを初期化する。
	/// </summary>
	void Initialize(DirectXCommon *dxCommon,
	                GPUParticlePipeline *pipeline,
	                TextureManager *textureManager,
	                uint32_t textureHandle);
	/// <summary>
	/// 各ミサイルの現在位置へ、経過時間に応じた数の煙を発生させる。
	/// </summary>
	void EmitTrails(const std::vector<Vector3> &missilePositions, float deltaTime);
	/// <summary>
	/// 発生済みの煙パーティクルを経過時間分進める。
	/// </summary>
	void Update(float deltaTime);
	/// <summary>
	/// カメラを基準に煙パーティクルを描画する。
	/// </summary>
	void Draw(const Camera &camera);
	/// <summary>
	/// 残存パーティクルと発生タイマーを初期状態へ戻す。
	/// </summary>
	void Reset();

  private:
	GPUParticleSystem particleSystem_{};
	float emissionAccumulator_ = 0.0f; // 1個未満の発生量を次フレームへ持ち越す
	uint32_t emissionSequence_ = 0;    // 発生ごとに乱数系列を変えるための通し番号
	bool initialized_ = false;         // GPUリソースを利用できる状態か
	static constexpr float kParticlesPerSecondPerMissile = 420.0f;
};
