#pragma once

#include "engine/graphics/resource/SrvManager.h"

#include <d3d12.h>
#include <wrl.h>

#include <array>

class DirectXCommon;
class ShaderCompiler;

/// <summary>
/// World描画とポストエフェクトで交互に使う2枚のRenderTextureを管理する。
/// Effectの種類や順序は扱わず、GPU Resource、View、状態遷移だけを担当する。
/// </summary>
class RenderTargetManager final {
  public:
	static constexpr uint32_t kRenderTextureCount = 2;
	static constexpr uint32_t kWorldTextureIndex = 0;

	RenderTargetManager() = default;
	~RenderTargetManager();

	RenderTargetManager(const RenderTargetManager &) = delete;
	RenderTargetManager &operator=(const RenderTargetManager &) = delete;

	/// <summary>
	/// Ping-Pong用Texture2枚、World深度、最終コピー用Pipelineを生成する。
	/// </summary>
	void Initialize(DirectXCommon *directXCommon, SrvManager *srvManager, ShaderCompiler *shaderCompiler);

	/// <summary>
	/// World描画先をRenderTextureへ切り替え、カラーと深度をクリアする。
	/// </summary>
	void BeginWorldPass();

	/// <summary>
	/// 指定Textureをポストエフェクト出力先にし、深度なしで描画を開始する。
	/// </summary>
	void BeginPostEffectPass(uint32_t textureIndex);

	/// <summary>
	/// 指定TextureをPixel Shaderから読み取れる状態へ遷移する。
	/// </summary>
	void PrepareForSampling(uint32_t textureIndex);

	/// <summary>
	/// 指定TextureのSRVを、ポストエフェクトへ渡せるGPUハンドルとして取得する。
	/// </summary>
	[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrv(uint32_t textureIndex) const;

	/// <summary>
	/// 指定Textureを入力として、World結果をSwapChainへコピーする。
	/// </summary>
	void CompositeToSwapChain(uint32_t sourceTextureIndex);

	/// <summary>
	/// 確保したSRVスロットとGPUリソースを解放する。
	/// </summary>
	void Finalize() noexcept;

  private:
	/// <summary>
	/// 画面サイズと同じカラーRenderTextureを2枚生成する。
	/// </summary>
	void CreateColorTextures();

	/// <summary>
	/// World描画専用の深度ステンシルを生成する。
	/// </summary>
	void CreateDepthTexture();

	/// <summary>
	/// 全画面三角形でRenderTextureをコピーするPSOを生成する。
	/// </summary>
	void CreateCompositePipeline();

	/// <summary>
	/// 指定TextureをRTV状態へ遷移し、深度使用条件に合わせて描画先へ設定する。
	/// </summary>
	void BeginColorPass(uint32_t textureIndex, bool useDepth);

	/// <summary>
	/// Texture番号がPing-Pong配列内に収まっていることを検証する。
	/// </summary>
	void ValidateTextureIndex(uint32_t textureIndex) const;

	DirectXCommon *directXCommon_ = nullptr; // DirectXデバイスとコマンドリストへの非所有参照
	SrvManager *srvManager_ = nullptr;       // SRV割り当て機能への非所有参照
	ShaderCompiler *shaderCompiler_ = nullptr;   // HLSLコンパイラへの非所有参照

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kRenderTextureCount> colorTextures_{}; // Ping-PongカラーTexture
	Microsoft::WRL::ComPtr<ID3D12Resource> depthTexture_; // World描画専用の深度Texture
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_; // カラーTexture2枚分のRTVヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_; // depthTexture_用DSVヒープ
	std::array<SrvHandle, kRenderTextureCount> colorSrvs_{}; // カラーTexture2枚分のSRV
	uint32_t rtvDescriptorSize_ = 0;                         // RTV間のバイト間隔

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_; // 全画面コピー用RootSignature
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;  // 全画面コピー用PSO

	std::array<D3D12_RESOURCE_STATES, kRenderTextureCount> colorStates_ = { // CPU側で追跡する現在状態
	    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	};
	bool initialized_ = false;
};
