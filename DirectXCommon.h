#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "WinApp.h"

/// <summary>
/// DirectX基盤
/// </summary>

class DirectXCommon
{
public:
	//====================
	// メンバ関数
	//====================

	// 初期化
	void Initialize(WinApp* winApp);

private:
	//====================
	// メンバ変数
	//====================

	// デバイスの生成
	void CreateDevice();

	// コマンド関連の生成
	void CreateCommandObjects();

	// スワップチェーン生成
	void CreateSwapChain();

	// 深度バッファの生成
	void CreateDepthStencilBuffer();

	// 各デスクリプタヒープの生成
	void CreateDescriptorHeap();

	// レンダーターゲットビュー
	void CreateRenderTargetView();

	// 深度ステンシルビュー
	void CreateDepthStencilView();

	// フェンス
	void CreateFence();

	// ビューポート
	void CreateViewPort();

	// シザリング矩形


	// DXCコンパイラ


	// ImGui


	// WindowsAPI
	WinApp* winApp = nullptr;

private:
	//====================
	// 名前空間
	//====================

	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;

	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	// デスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
};

