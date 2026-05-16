#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "DirectXCommon.h"

class Object3dCommon {
public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 描画前の共通設定
	/// </summary>
	void CommonDrawSetting();

public:
	/// <summary>
	/// DirectXCommonクラスのゲッター
	/// </summary>
	/// <returns></returns>
	DirectXCommon* GetDxCommon() const { return dxCommon_; }

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
	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
};