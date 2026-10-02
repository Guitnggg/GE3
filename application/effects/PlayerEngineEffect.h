#pragma once

#include "engine/effects/particle/GPUParticleEmitter.h"
#include "engine/effects/particle/GPUParticleSystem.h"

#include <cstdint>

class Camera;
class DirectXCommon;
class TextureManager;

/// <summary>
/// プレイヤーエンジン噴射の調整値。
/// </summary>
struct PlayerEngineEffectSettings {
	bool enabled = true;
	uint32_t count = 16;
	float interval = 0.006f;
	float minSpeed = 0.0f;
	float maxSpeed = 16.0f;
	float velocitySpread = 0.0f;
	float positionSpread = 0.0f;
	float minLifetime = 0.20f;
	float maxLifetime = 0.05f;
	float startSize = 0.40f;
	float endSize = 0.0f;
	Vector4 startColor{0.65f, 0.9f, 1.0f, 0.95f};
	Vector4 endColor{0.05f, 0.2f, 1.0f, 0.0f};
	float accelerationZ = -5.0f;
	float drag = 0.8f;
	Vector3 nozzleOffset{0.0f, -0.15f, -0.72f};
};

/// <summary>
/// 中央噴射口の設定反映、発生、更新、描画をまとめる。
/// </summary>
class PlayerEngineEffect final {
public:
	void Initialize(DirectXCommon* dxCommon, TextureManager* textureManager, uint32_t textureHandle,
		const PlayerEngineEffectSettings& settings);
	void Reset(const PlayerEngineEffectSettings& settings);
	void Update(float deltaTime, const Vector3& playerPosition, const PlayerEngineEffectSettings& settings);
	void Draw(const Camera& camera);

private:
	void ApplySettings(const PlayerEngineEffectSettings& settings);

	GPUParticleSystem particleSystem_{};
	GPUParticleEmitter emitter_{};
	bool initialized_ = false;
};
