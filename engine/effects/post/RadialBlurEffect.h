#pragma once

#include "engine/effects/post/IPostEffect.h"

#include "engine/math/MathTypes.h"

#include <cstdint>
#include <wrl.h>

class DirectXCommon;
class ShaderCompiler;

/// <summary>
/// 指定した画面座標へ向かって複数回TextureをSamplingし、放射状の残像を作るEffect。
/// Sampling回数とBlur幅を実行時に変更でき、Ping-Pongの1 Passとして動作する。
/// </summary>
class RadialBlurEffect final : public IPostEffect {
  public:
	static constexpr uint32_t kMinSampleCount = 2;
	static constexpr uint32_t kMaxSampleCount = 32;

	/// <summary>
	/// Fullscreen Shader、RootSignature、PSOを生成する。
	/// </summary>
	void Initialize(DirectXCommon *directXCommon, ShaderCompiler *shaderCompiler) override;

	/// <summary>
	/// PostEffectManagerがこのEffectを処理チェーンへ含めるか返す。
	/// </summary>
	[[nodiscard]] bool IsEnabled() const override { return enabled_; }

	/// <summary>
	/// 入力Textureを放射方向へSamplingし、設定済みRenderTargetへ描画する。
	/// </summary>
	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE sourceTexture) override;

	/// <summary>
	/// 有効状態、中心座標、Blur幅、Sampling回数をImGuiから調整する。
	/// </summary>
	void DrawDebugUI() override;

	void SetEnabled(bool enabled) { enabled_ = enabled; }
	void SetCenter(const Vector2 &center);
	void SetBlurWidth(float blurWidth);
	void SetSampleCount(uint32_t sampleCount);

	[[nodiscard]] Vector2 GetCenter() const { return parameters_.center; }
	[[nodiscard]] float GetBlurWidth() const { return parameters_.blurWidth; }
	[[nodiscard]] uint32_t GetSampleCount() const { return parameters_.sampleCount; }

  private:
	/// <summary>
	/// Pixel Shaderへ毎フレーム渡すRadial Blur調整値。
	/// HLSL側のcbufferと同じ16-byte配置を維持する。
	/// </summary>
	struct Parameters {
		Vector2 center{0.5f, 0.5f}; // 放射線が収束するUV座標
		float blurWidth = 0.08f;    // 中心方向へSamplingする最大距離の倍率
		uint32_t sampleCount = 12;  // 1 PixelあたりのTexture Sampling回数
	};

	void CreateRootSignature();
	void CreatePipelineState();

	DirectXCommon *directXCommon_ = nullptr;   // CommandListと動的CB確保への非所有参照
	ShaderCompiler *shaderCompiler_ = nullptr; // Shaderコンパイル機能への非所有参照
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Parameters parameters_{};
	bool enabled_ = true;
	bool initialized_ = false;
};
