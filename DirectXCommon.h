#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "WinApp.h"

#include "externals/DirectXTex/DirectXTex.h"

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

	// デスクリプタヒープを生成する
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	// SRVの指定番号のCPUデスクリプタハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	// SRVの指定番号のGPUデスクリプタハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

private:
	//====================
	// メンバ関数（内部処理専用関数）
	//====================

	// 指定番号のCPUデスクリプタハンドルを取得する
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	// 指定番号のGPUデスクリプタハンドルを取得する
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

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
	void CreateScissorRect();

	// DXCコンパイラ
	void CreateDXCCompiler();

	// ImGui
	void CreateImGui();

	// WindowsAPI
	WinApp* winApp = nullptr;

private:
	//====================
	// 名前空間（ローカル変数からメンバ変数に）
	//====================

	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;

	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	// コマンド関連
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue;

	// スワップチェーン
	Microsoft::WRL::ComPtr < IDXGISwapChain4> swapChain;

	// 
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2>swapChainResources;

	
};

