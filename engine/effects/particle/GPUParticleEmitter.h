#pragma once

#include "engine/effects/particle/GPUParticleCommon.h"

#include <cstdint>

class GPUParticleSystem;

/// <summary>継続発生のタイミングとEmitテンプレートを保持する、軽量なCPU側エミッター。</summary>
class GPUParticleEmitter final {
public:
	void Initialize(GPUParticleSystem* system, const GPUParticleEmitData& emitTemplate,
		float intervalSeconds);
	void Update(float deltaTime, const Vector3& position);
	void EmitOnce(const Vector3& position);
	void Reset();
	void SetEmitTemplate(const GPUParticleEmitData& emitTemplate) { emitTemplate_ = emitTemplate; }
	void SetInterval(float intervalSeconds);
	void SetActive(bool active) { isActive_ = active; }
	bool IsActive() const { return isActive_; }

private:
	GPUParticleSystem* system_ = nullptr;
	GPUParticleEmitData emitTemplate_{};
	float interval_ = 0.0f;
	float elapsed_ = 0.0f;
	uint32_t emissionSequence_ = 0;
	bool isActive_ = false;
};
