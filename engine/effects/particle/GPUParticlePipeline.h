#pragma once

#include "engine/effects/particle/GPUParticleCommon.h"

#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;
class ShaderCompiler;

/// <summary>
/// 全GPUパーティクルシステムで共有するRootSignatureとPSOを所有する。
/// </summary>
class GPUParticlePipeline final {
  public:
	/// <summary>
	/// 共有ルートシグネチャと全コンピュート・描画PSOを生成する。
	/// </summary>
	void Initialize(DirectXCommon *dxCommon, ShaderCompiler *shaderCompiler);

	/// <summary>
	/// 初期化、発生、更新で共有するコンピュートルートシグネチャを取得する。
	/// </summary>
	ID3D12RootSignature *GetComputeRootSignature() const {
		return computeRootSignature_.Get();
	}
	/// <summary>
	/// パーティクル描画用ルートシグネチャを取得する。
	/// </summary>
	ID3D12RootSignature *GetGraphicsRootSignature() const {
		return graphicsRootSignature_.Get();
	}
	/// <summary>
	/// パーティクルバッファ初期化用PSOを取得する。
	/// </summary>
	ID3D12PipelineState *GetInitializePipeline() const {
		return initializePipeline_.Get();
	}
	/// <summary>
	/// パーティクル発生用PSOを取得する。
	/// </summary>
	ID3D12PipelineState *GetEmitPipeline() const {
		return emitPipeline_.Get();
	}
	/// <summary>
	/// パーティクル更新用PSOを取得する。
	/// </summary>
	ID3D12PipelineState *GetUpdatePipeline() const {
		return updatePipeline_.Get();
	}
	/// <summary>
	/// 指定したブレンド方式に対応する描画PSOを取得する。
	/// </summary>
	ID3D12PipelineState *GetGraphicsPipeline(GPUParticleBlendMode blendMode) const;

  private:
	// 初期化処理をリソースの種類ごとに分割し、生成順序を明確にする。
	void CreateRootSignatures();
	void CreateComputePipelines();
	void CreateGraphicsPipelines();

	DirectXCommon *dxCommon_ = nullptr;
	ShaderCompiler *shaderCompiler_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> graphicsRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> initializePipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> emitPipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> updatePipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> alphaPipeline_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> additivePipeline_;
};
