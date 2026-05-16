#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "engine/base/DirectXCommon.h"

/// <summary>
/// スプライト描画で共有するルートシグネチャとパイプラインを管理するクラス
/// </summary>
class SpriteCommon {
public:
	/// <summary>
	/// DirectX共通処理を受け取り、スプライト描画用の共通設定を初期化する
	/// </summary>
	/// <param name="dxCommon">DirectX共通処理</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// スプライト描画前に共通の描画設定をコマンドリストへ設定する
	/// </summary>
	void CommonDrawSetting();

	/// <summary>
	/// DirectXCommonクラスを取得する
	/// </summary>
	/// <returns>DirectX共通処理</returns>
	DirectXCommon* GetDXCommon() const { return dxCommon_; }

private:
	/// <summary>
	/// スプライト描画用のルートシグネチャを生成する
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// スプライト描画用のグラフィックスパイプラインを生成する
	/// </summary>
	void CreateGraphicsPipeline();

private:
	DirectXCommon* dxCommon_ = nullptr;  // DirectX共通処理

	// ===== パイプライン関連 =====
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;          // ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;  // グラフィックスパイプラインステート
};
