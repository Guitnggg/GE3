#pragma once

#include <chrono>
#include <d3d12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <format>
#include <wrl.h>

#include "WinApp.h"
#include "StringUtility.h"
#include "Logger.h"

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

/// <summary>
/// DirectX 12の初期化、描画開始終了、GPUリソース生成をまとめて管理するクラス
/// </summary>
class DirectXCommon {
public:
	/// <summary>
	/// DirectX関連の終了処理を行う
	/// </summary>
	~DirectXCommon();

	/// <summary>
	/// DirectX 12で描画するための各種オブジェクトを初期化する
	/// </summary>
	/// <param name="winApp">Windowsアプリケーション</param>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// 深度ステンシル用テクスチャリソースを生成する
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(
		Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	/// <summary>
	/// 指定された種類のディスクリプタヒープを生成する
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		Microsoft::WRL::ComPtr<ID3D12Device> device,
		D3D12_DESCRIPTOR_HEAP_TYPE heapType,
		UINT numDesciptors,
		bool shaderVisible);

	/// <summary>
	/// 指定番号のCPUディスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// 指定番号のGPUディスクリプタハンドルを取得する
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// SRV用のCPUディスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleSRV(uint32_t index);

	/// <summary>
	/// SRV用のGPUディスクリプタハンドルを取得する
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleSRV(uint32_t index);
	ID3D12DescriptorHeap* GetSRVDescriptorHeap() const { return srvDescriptorHeap.Get(); }
	uint32_t GetSRVDescriptorSize() const { return descriptorSizeSRV; }
	uint32_t GetSwapChainBufferCount() const { return swapChainDesc.BufferCount; }
	DXGI_FORMAT GetRenderTargetFormat() const { return rtvDesc.Format; }

	/// <summary>
	/// Direct3Dデバイスを取得する
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device.Get(); }

	/// <summary>
	/// 描画コマンドリストを取得する
	/// </summary>
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }

	/// <summary>
	/// HLSLシェーダーをコンパイルする
	/// </summary>
	IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile);

	/// <summary>
	/// CPUから書き込めるアップロード用バッファリソースを生成する
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	/// <summary>
	/// テクスチャ用リソースを生成する
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CrateTextureResource(
		Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata);

	/// <summary>
	/// ScratchImageのミップマップデータをテクスチャリソースへ転送する
	/// </summary>
	void UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);

	/// <summary>
	/// 画像ファイルを読み込み、ミップマップ付きテクスチャデータを生成する
	/// </summary>
	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	/// <summary>
	/// 1フレーム分の描画前処理を行う
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 1フレーム分の描画後処理を行う
	/// </summary>
	void PostDraw();

private:
	// ===== 初期化用メンバ関数 =====

	void CreateInitialze();       // デバイスとDXGIファクトリを生成する
	void CreateCommand();         // コマンドキュー、アロケータ、リストを生成する
	void CreateSwapChain();       // スワップチェーンを生成する
	void CreateDepthBuffer();     // 深度バッファを生成する
	void CreateDescritorHeap();   // 各種ディスクリプタヒープを生成する
	void CreateRenderTargetView();// レンダーターゲットビューを生成する
	void CreateDepthStencilView();// 深度ステンシルビューを生成する
	void CreateFence();           // GPU同期用フェンスを生成する
	void CreateViewport();        // ビューポートを設定する
	void CreateScissorRect();     // シザー矩形を設定する
	void CreateDXC();             // DXCコンパイラ関連を初期化する

	// ===== FPS固定処理 =====

	void CreateFixFPS();  // FPS固定用の基準時間を初期化する
	void UpdateFixFPS();  // 60FPSになるよう待機時間を調整する

private:
	// ===== アプリケーション関連 =====

	WinApp* winApp = nullptr;  // Windowsアプリケーション

	// ===== DirectX基本オブジェクト =====

	Microsoft::WRL::ComPtr<ID3D12Device> device;                       // Direct3Dデバイス
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;                 // DXGIファクトリ
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;   // コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;           // コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;     // コマンドリスト

	// ===== スワップチェーン関連 =====

	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;                 // スワップチェーン
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};                             // スワップチェーン設定
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2];      // バックバッファ
	D3D12_RESOURCE_BARRIER barrier{};                                  // リソース状態遷移用バリア

	// ===== ディスクリプタ関連 =====

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;    // SRV用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;    // RTV用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;    // DSV用ディスクリプタヒープ
	uint32_t descriptorSizeSRV = 0;                                    // SRVディスクリプタサイズ
	uint32_t descriptorSizeRTV = 0;                                    // RTVディスクリプタサイズ
	uint32_t descriptorSizeDSV = 0;                                    // DSVディスクリプタサイズ
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2]{};                       // RTVハンドル
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};                           // RTV設定

	// ===== 深度バッファ関連 =====

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;        // 深度ステンシルリソース

	// ===== GPU同期関連 =====

	Microsoft::WRL::ComPtr<ID3D12Fence> fence;                         // GPU同期用フェンス
	HANDLE fenceEvent = nullptr;                                       // フェンス完了待ちイベント
	uint64_t fenceValue = 0;                                           // フェンス値

	// ===== 描画領域 =====

	D3D12_VIEWPORT viewport{};                                         // ビューポート
	D3D12_RECT scissorRect{};                                          // シザー矩形

	// ===== シェーダーコンパイル関連 =====

	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;                        // DXCユーティリティ
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;                 // DXCコンパイラ
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;         // include解決用ハンドラ

	// ===== FPS固定用 =====

	std::chrono::steady_clock::time_point reference_;                  // 前回フレームの基準時間
};
