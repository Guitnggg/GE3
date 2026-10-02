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
	void Initialize(DirectXCommon* dxCommon, GPUParticlePipeline* pipeline,
		TextureManager* textureManager, uint32_t textureHandle);
	void EmitTrails(const std::vector<Vector3>& missilePositions, float deltaTime);
	void Update(float deltaTime);
	void Draw(const Camera& camera);
	void Reset();

private:
	GPUParticleSystem particleSystem_{};
	float emissionAccumulator_ = 0.0f;
	uint32_t emissionSequence_ = 0;
	bool initialized_ = false;
	static constexpr float kParticlesPerSecondPerMissile = 420.0f;
};
