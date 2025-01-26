#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <format>
#include <dxcapi.h>

#include "WinApp.h"
#include "StringUtility.h"
#include "Logger.h"

#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

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

	// DepthStencilTextureの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr <ID3D12Device> device, int32_t width, int32_t height);

	// デスクリプタヒープの生成
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr <ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDesciptors, bool shaderVisible);

	// 指定番号のCPUデスクリプタハンドルを取得
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	// 指定番号のGPUデスクリプタハンドルを取得
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	// 
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleSRV(uint32_t index);

	// 
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleRTV(uint32_t index);

private:

	// デバイスの初期化
	void CreateInitialze();

	// コマンド関連の初期化
	void CreateCommand();

	// スワップチェーンの初期化
	void CreateSwapChain();

	// 深度バッファの初期化
	void CreateDepthBuffer();

	// 各種デスクリプタヒープの初期化
	void CreateDescritorHeap();

	// レンダーターゲットビューの初期化
	void CreateRenderTargetView();

	// 深度ステンシルビューの初期化
	void CreateDepthStencilView();

	// フェンスの初期化
	void CreateFence();

	// ビューポートの初期化
	void CreateViewport();

	// シザリング矩形の初期化
	void CreateScissorRect();

	// DXCの初期化
	void CreateDXC();

	// ImGuiの初期化
	void CreateImGui();

private:

	//====================
	// メンバ変数
	//====================

	WinApp* winApp = nullptr;

	// デバイス
	Microsoft::WRL::ComPtr <ID3D12Device> device;

	// DXGIファクトリ
	Microsoft::WRL::ComPtr <IDXGIFactory7> dxgiFactory;

	// コマンドアロケータ
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator;

	// コマンドキュー
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue;

	// コマンドリスト	
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList;

	// スワップチェイン
	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;

	// SRVデスクリプタヒープ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeapSRV;

	// RTVデスクリプタヒープ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeapRTV;

	// DSVデスクリプタヒープ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeapDSV;

	// 各種デスクリプタヒープ
	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;

	// SRVのデスクリプタハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	
	// スワップチェーンリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2];

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;
	
	// フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	HANDLE fenceEvent;

	// ビューポート
	D3D12_VIEWPORT viewport{};
	// シザー矩形
	D3D12_RECT scissorRect{};

};

