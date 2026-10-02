#pragma once

#include "engine/effects/particle/GPUParticleCommon.h"

#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;

/// <summary>全GPUパーティクルシステムで共有するRootSignatureとPSOを所有する。</summary>
class GPUParticlePipeline final {
public:
	void Initialize(DirectXCommon* dxCommon);

	ID3D12RootSignature* GetComputeRootSignature() const { return computeRootSignature_.Get(); }
	ID3D12RootSignature* GetGraphicsRootSignature() const { return graphicsRootSignature_.Get(); }
	ID3D12PipelineState* GetInitializePipeline() const { return initializePipeline_.Get(); }
	ID3D12PipelineState* GetEmitPipeline() const { return emitPipeline_.Get(); }
	ID3D12PipelineState* GetUpdatePipeline() const { return updatePipeline_.Get(); }
	ID3D12PipelineState* GetGraphicsPipeline(GPUParticleBlendMode blendMode) const;

private:
	void CreateRootSignatures();
	void CreateComputePipelines();
	void CreateGraphicsPipelines();

	DirectXCommon* dxCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> graphicsRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> initializePipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> emitPipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> updatePipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> alphaPipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> additivePipeline_;
};
