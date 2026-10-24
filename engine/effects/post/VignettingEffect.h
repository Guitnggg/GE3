#pragma once

#include "engine/effects/post/IPostEffect.h"

#include <wrl.h>

class DirectXCommon;
class ShaderCompiler;

/// <summary>
/// 画面中央を残しながら、端へ近づくほど色を暗くするVignetting Effect。
/// Effect固有のPipelineと調整値を所有し、Ping-Pongの1 Passとして動作する。
/// </summary>
class VignettingEffect final : public IPostEffect {
  public:
	/// <summary>
	/// Fullscreen Shader、RootSignature、PSOを生成する。
	/// </summary>
	void Initialize(DirectXCommon *directXCommon, ShaderCompiler *shaderCompiler) override;

	/// <summary>
	/// PostEffectManagerがこのEffectを処理チェーンへ含めるか返す。
	/// </summary>
	[[nodiscard]] bool IsEnabled() const override { return enabled_; }

	/// <summary>
	/// 入力Textureを読み、設定済みRenderTargetへVignetting結果を描画する。
	/// </summary>
	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE sourceTexture) override;

	void SetEnabled(bool enabled) { enabled_ = enabled; }
	void SetIntensity(float intensity);
	void SetPower(float power);

	[[nodiscard]] float GetIntensity() const { return parameters_.intensity; }
	[[nodiscard]] float GetPower() const { return parameters_.power; }

  private:
	/// <summary>
	/// Pixel Shaderへ毎フレーム渡すVignetting調整値。
	/// </summary>
	struct Parameters {
		float intensity = 16.0f; // 暗くなり始める範囲。大きいほど明るい範囲が広がる
		float power = 0.8f;      // 中央から端へ落ちるカーブ。大きいほど端が強く暗くなる
		float padding[2]{};      // Constant Bufferの16-byte境界を明示する余白
	};

	/// <summary>
	/// 入力SRVとパラメータCBVを受け取るRootSignatureを生成する。
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// Fullscreen Vertex ShaderとVignetting Pixel ShaderからPSOを生成する。
	/// </summary>
	void CreatePipelineState();

	DirectXCommon *directXCommon_ = nullptr; // コマンド記録と動的CB確保への非所有参照
	ShaderCompiler *shaderCompiler_ = nullptr; // Shaderコンパイル機能への非所有参照
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Parameters parameters_{};
	bool enabled_ = true;
	bool initialized_ = false;
};
