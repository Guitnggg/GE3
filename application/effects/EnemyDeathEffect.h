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
	void Initialize(DirectXCommon* dxCommon, GPUParticlePipeline* pipeline,
		TextureManager* textureManager, uint32_t textureHandle);
	void Emit(const Vector3& position);
	void Update(float deltaTime);
	void Draw(const Camera& camera);
	void Reset();

private:
	GPUParticleSystem flashSystem_{}; // 爆発直後に膨らむ白熱コア
	GPUParticleSystem sparkSystem_{}; // 外側へ高速で飛散する火花
	GPUParticleSystem smokeSystem_{}; // 遅れて膨らみながら残る煙
	uint32_t emissionSequence_ = 0;
	bool initialized_ = false;
};
