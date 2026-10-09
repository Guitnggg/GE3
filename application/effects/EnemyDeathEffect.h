#pragma once

#include "engine/effects/particle/GPUParticleSystem.h"

#include <cstdint>

class Camera;
class DirectXCommon;
class GPUParticlePipeline;
class TextureManager;

/// <summary>
/// 敵撃破時の単発GPUパーティクル爆発を管理する。
/// </summary>
class EnemyDeathEffect final {
  public:
	/// <summary>
	/// 爆発を構成する白熱、火花、煙のパーティクルシステムを初期化する。
	/// </summary>
	void Initialize(DirectXCommon *dxCommon,
	                GPUParticlePipeline *pipeline,
	                TextureManager *textureManager,
	                uint32_t textureHandle);
	/// <summary>
	/// 指定したワールド座標に撃破エフェクトを1回発生させる。
	/// </summary>
	void Emit(const Vector3 &position);
	/// <summary>
	/// すべての撃破パーティクルを経過時間分進める。
	/// </summary>
	void Update(float deltaTime);
	/// <summary>
	/// カメラを基準に撃破パーティクルを描画する。
	/// </summary>
	void Draw(const Camera &camera);
	/// <summary>
	/// 残存パーティクルと発生順序を初期状態へ戻す。
	/// </summary>
	void Reset();

  private:
	GPUParticleSystem flashSystem_{}; // 爆発直後に膨らむ白熱コア
	GPUParticleSystem sparkSystem_{}; // 外側へ高速で飛散する火花
	GPUParticleSystem smokeSystem_{}; // 遅れて膨らみながら残る煙
	uint32_t emissionSequence_ = 0;   // 発生ごとに乱数系列を変えるための通し番号
	bool initialized_ = false;        // GPUリソースを利用できる状態か
};
