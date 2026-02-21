#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "DirectXCommon.h"

/// <summary>
/// 
/// </summary>
class SpriteCommon{
public:
	/// <summary>
	/// 
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 
	/// </summary>
	void CommonDrawSetting();

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	DirectXCommon* GetDXCommon() const { return dxCommon_; }

private:
	/// <summary>
	/// 
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// 
	/// </summary>
	void CreateGraphicsPipeline();

private:
	// 
	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;          // ルートシグネチャー
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;  // グラフィックスパイプラインステート
};
