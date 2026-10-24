#pragma once

#include "engine/effects/post/IPostEffect.h"

#include <memory>
#include <vector>

class DirectXCommon;
class RenderTargetManager;
class ShaderCompiler;

/// <summary>
/// 登録されたポストエフェクトを順番に実行し、2枚のRenderTextureを交互に使用する。
/// エフェクトが0個の場合も、World TextureをそのままSwapChainへ合成する。
/// </summary>
class PostEffectManager final {
  public:
	/// <summary>
	/// Ping-Pong対象と、エフェクト初期化に必要な描画サービスを受け取る。
	/// </summary>
	void Initialize(DirectXCommon *directXCommon,
	                RenderTargetManager *renderTargetManager,
	                ShaderCompiler *shaderCompiler);

	/// <summary>
	/// エフェクトの所有権を受け取り、登録順に処理するチェーンへ追加する。
	/// </summary>
	void AddEffect(std::unique_ptr<IPostEffect> effect);

	/// <summary>
	/// 有効なエフェクトをPing-Pong実行し、最終結果をSwapChainへ合成する。
	/// </summary>
	void Apply();

	/// <summary>
	/// 登録したエフェクトと外部サービスへの参照を解放する。
	/// </summary>
	void Finalize() noexcept;

  private:
	DirectXCommon *directXCommon_ = nullptr;             // コマンド記録先への非所有参照
	RenderTargetManager *renderTargetManager_ = nullptr; // Ping-Pong Texture管理への非所有参照
	ShaderCompiler *shaderCompiler_ = nullptr;           // Effect初期化用Compilerへの非所有参照
	std::vector<std::unique_ptr<IPostEffect>> effects_;  // 登録順に所有するEffect一覧
	bool initialized_ = false;
};
