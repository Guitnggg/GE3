#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "engine/base/DirectXCommon.h"

/// <summary>
/// スプライト共通
/// </summary>
class SpriteCommon{
public:  // メンバ関数
	/// <summary>
	///　初期化処理
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	///　描画前の共通設定
	/// </summary>
	void CommonDrawSetting();

	/// <summary>
	///　DirectXCommonのゲッター
	/// </summary>
	/// <returns></returns>
	DirectXCommon* GetDXCommon() const { return dxCommon_; }

private:
	/// <summary>
	/// ルートシグネチャーの生成
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// グラフィックスパイプラインの生成
	/// </summary>
	void CreateGraphicsPipeline();

private:
	// 
	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;          // ルートシグネチャー
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;  // グラフィックスパイプラインステート
};
