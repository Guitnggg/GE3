#pragma once

#include "engine/effects/particle/GPUParticleCommon.h"

#include <cstdint>
#include <vector>

#include <d3d12.h>
#include <wrl.h>

class Camera;
class DirectXCommon;
class TextureManager;

/// <summary>
/// GPU上で生成・更新・描画を完結する再利用可能なパーティクルプール。
/// 演出差分は継承ではなくPresetとEmitDataで与える。
/// </summary>
class GPUParticleSystem final {
public:
	void Initialize(DirectXCommon* dxCommon, TextureManager* textureManager,
		uint32_t textureHandle, uint32_t maxParticles = 4096);
	void SetPreset(const GPUParticlePreset& preset);
	void Emit(const GPUParticleEmitData& emitData);
	void Update(float deltaTime);
	void Draw(const Camera& camera);
	void Reset();

	uint32_t GetMaxParticles() const { return maxParticles_; }
	const GPUParticlePreset& GetPreset() const { return preset_; }

private:
	static constexpr uint32_t kThreadGroupSize = 256;
	static constexpr uint32_t kMaxEmitCommandsPerFrame = 64;

	struct alignas(16) Particle {
		Vector3 position{};
		float lifetime = 0.0f;
		Vector3 velocity{};
		float age = 0.0f;
		Vector4 startColor{};
		Vector4 endColor{};
		Vector2 startSize{};
		Vector2 endSize{};
	};

	struct alignas(256) SimulationConstants {
		float deltaTime = 0.0f;
		uint32_t maxParticles = 0;
		Vector2 padding{};
		Vector3 acceleration{};
		float drag = 0.0f;
	};

	struct alignas(256) EmitConstants {
		Vector3 position{};
		uint32_t count = 0;
		Vector3 minVelocity{};
		uint32_t seed = 0;
		Vector3 maxVelocity{};
		float minLifetime = 0.0f;
		Vector3 positionSpread{};
		float maxLifetime = 0.0f;
		Vector4 startColor{};
		Vector4 endColor{};
		Vector2 startSize{};
		Vector2 endSize{};
	};

	struct alignas(256) DrawConstants {
		Matrix4x4 viewProjection{};
		Vector3 cameraRight{};
		float padding0 = 0.0f;
		Vector3 cameraUp{};
		float padding1 = 0.0f;
	};

	void CreateResources();
	void CreateComputePipeline();
	void CreateGraphicsPipeline();
	void CreateRootSignatures();
	void TransitionParticleBuffer(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	void DispatchInitialize();
	void DispatchEmitCommands();
	void DispatchUpdate();

	DirectXCommon* dxCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	uint32_t textureHandle_ = 0;
	uint32_t maxParticles_ = 0;
	float deltaTime_ = 0.0f;
	bool needsInitialize_ = true;
	bool simulationBuffersAreCommon_ = true;
	bool particleBufferIsSrv_ = false;
	GPUParticlePreset preset_{};
	std::vector<GPUParticleEmitData> pendingEmits_;

	Microsoft::WRL::ComPtr<ID3D12Resource> particleResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> freeListResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> freeListIndexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> simulationConstantResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> emitConstantResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> drawConstantResource_;
	SimulationConstants* simulationConstants_ = nullptr;
	EmitConstants* emitConstants_ = nullptr;
	DrawConstants* drawConstants_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> graphicsRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> initializePipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> emitPipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> updatePipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> alphaPipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> additivePipeline_;
};
