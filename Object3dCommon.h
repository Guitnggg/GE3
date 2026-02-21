#pragma once

#include "DirectXCommon.h"

class Object3dCommon {
public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(DirectXCommon* dxCommon);

public:
	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

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
	DirectXCommon* dxCommon_;

};