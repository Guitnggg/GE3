#pragma once

#include <d3d12.h>

class DirectXCommon;
class ShaderCompiler;

/// <summary>
/// Ping-Pong処理へ登録できる、1回分のフルスクリーンエフェクトを表す。
/// 各実装は入力SRVを読み、PostEffectManagerが設定した出力RTVへ描画する。
/// </summary>
class IPostEffect {
  public:
	virtual ~IPostEffect() = default;

	/// <summary>
	/// エフェクト固有のRootSignature、Shader、PSOを生成する。
	/// </summary>
	virtual void Initialize(DirectXCommon *directXCommon, ShaderCompiler *shaderCompiler) = 0;

	/// <summary>
	/// 現在このエフェクトを処理チェーンへ含めるか返す。
	/// </summary>
	[[nodiscard]] virtual bool IsEnabled() const = 0;

	/// <summary>
	/// sourceTextureを入力として、現在設定済みのRTVへ全画面描画する。
	/// </summary>
	virtual void Draw(D3D12_GPU_DESCRIPTOR_HANDLE sourceTexture) = 0;
};
