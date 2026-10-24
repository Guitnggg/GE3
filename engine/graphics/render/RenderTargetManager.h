#pragma once

#include "engine/graphics/resource/SrvManager.h"

#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;
class ShaderCompiler;

/// <summary>
/// World描画用RenderTextureと、SwapChainへ転送する全画面描画を管理する。
/// DirectXCommonからオフスクリーン描画の責務を分離するための境界となるクラス。
/// </summary>
class RenderTargetManager final {
  public:
	RenderTargetManager() = default;
	~RenderTargetManager();

	RenderTargetManager(const RenderTargetManager &) = delete;
	RenderTargetManager &operator=(const RenderTargetManager &) = delete;

	/// <summary>
	/// RenderTexture、深度バッファ、全画面コピー用パイプラインを生成する。
	/// </summary>
	void Initialize(DirectXCommon *directXCommon, SrvManager *srvManager, ShaderCompiler *shaderCompiler);

	/// <summary>
	/// World描画先をRenderTextureへ切り替え、カラーと深度をクリアする。
	/// </summary>
	void BeginWorldPass();

	/// <summary>
	/// World描画結果をSwapChainへコピーし、後続のUI描画先をSwapChainへ戻す。
	/// </summary>
	void CompositeToSwapChain();

	/// <summary>
	/// 確保したSRVスロットとGPUリソースを解放する。
	/// </summary>
	void Finalize() noexcept;

  private:
	/// <summary>
	/// 画面サイズと同じカラーRenderTextureを生成する。
	/// </summary>
	void CreateColorTexture();

	/// <summary>
	/// World描画専用の深度ステンシルを生成する。
	/// </summary>
	void CreateDepthTexture();

	/// <summary>
	/// 全画面三角形でRenderTextureをコピーするPSOを生成する。
	/// </summary>
	void CreateCompositePipeline();

	DirectXCommon *directXCommon_ = nullptr; // DirectXデバイスとコマンドリストへの非所有参照
	SrvManager *srvManager_ = nullptr;       // SRV割り当て機能への非所有参照
	ShaderCompiler *shaderCompiler_ = nullptr; // HLSLコンパイラへの非所有参照

	Microsoft::WRL::ComPtr<ID3D12Resource> colorTexture_; // World描画結果を保持するカラーTexture
	Microsoft::WRL::ComPtr<ID3D12Resource> depthTexture_; // World描画専用の深度Texture
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_; // colorTexture_用RTVヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_; // depthTexture_用DSVヒープ
	SrvHandle colorSrv_{};                                  // Composite時に参照するカラーSRV

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_; // 全画面コピー用RootSignature
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;  // 全画面コピー用PSO

	D3D12_RESOURCE_STATES colorState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	bool initialized_ = false;
};
