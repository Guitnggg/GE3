#pragma once

#include "DirectXCommon.h"

#include <cassert>
#include <stdexcept>
#include <thread>

#include "externals/DirectXTex/DirectXTex.h"

using namespace Microsoft::WRL;

namespace {
void ThrowIfFailed(HRESULT result, const char* operation) {
	if (FAILED(result)) {
		throw std::runtime_error(std::string(operation) + " failed (HRESULT " + std::to_string(static_cast<unsigned long>(result)) + ").");
	}
}
}

//===============
// 初期化
//===============

// DirectXとImGuiで確保した終了処理を行う
DirectXCommon::~DirectXCommon()
{
	if (fenceEvent != nullptr) {
		CloseHandle(fenceEvent);
		fenceEvent = nullptr;
	}

	CoUninitialize();
}

// DirectX 12の描画に必要な各要素を順番に初期化する
void DirectXCommon::Initialize(WinApp* winApp)
{
	assert(winApp);

	CreateFixFPS();

	this->winApp = winApp;

	CreateInitialze();
	CreateCommand();
	CreateSwapChain();
	CreateDepthBuffer();
	CreateDescritorHeap();
	CreateRenderTargetView();
	CreateDepthStencilView();
	CreateFence();
	CreateViewport();
	CreateScissorRect();
	CreateDXC();
}

//===============
// その色々
//===============

// 深度ステンシル用テクスチャリソースを作成する
Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//二次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//DepthStrencil

	//利用するHeap
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAN上で作る


	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;


	//resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

// 指定した種類と数でディスクリプタヒープを作成する
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDesciptors, bool shaderVisible)
{
	//ディスクリプターヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDesciptors;

	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

// CPU用ディスクリプタハンドルを指定インデックス分進めて取得する
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

// GPU用ディスクリプタハンドルを指定インデックス分進めて取得する
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

// SRVヒープからCPU用ディスクリプタハンドルを取得する
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandleSRV(uint32_t index)
{
	return GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}

// SRVヒープからGPU用ディスクリプタハンドルを取得する
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandleSRV(uint32_t index)
{
	return GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}

// HLSLファイルをDXCでコンパイルする
IDxcBlob* DirectXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile)
{
	//1.hlslファイル
	Logger::Log(StringUtility::ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));

	Microsoft::WRL::ComPtr <IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	ThrowIfFailed(hr, "Loading a shader file");

	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	//2.Complie
	LPCWSTR arguments[] = {
		filePath.c_str(),
		L"-E",L"main",
		L"-T",profile,
		L"-Zi",L"-Qembed_debug",
		L"-Od",
		L"-Zpr",
	};

	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(&shaderSourceBuffer, arguments, _countof(arguments),
		includeHandler.Get(), IID_PPV_ARGS(&shaderResult));

	ThrowIfFailed(hr, "Compiling a shader");

	//3.警告エラー
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		Logger::Log(shaderError->GetStringPointer());
		//警告エラーダメ絶対
		throw std::runtime_error(std::string("Shader compilation failed: ") + shaderError->GetStringPointer());
	}

	//4.Complie結果
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	ThrowIfFailed(hr, "Getting compiled shader output");

	Logger::Log(StringUtility::ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));

	return shaderBlob;
}

// CPUから書き込めるバッファリソースを作成する
Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes)
{
	//VertexResource
	//頂点シェーダを作る
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vertexResourceDesc{};

	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;

	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	return vertexResource;
}

// 読み込んだ画像情報をもとにテクスチャリソースを作成する
Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CrateTextureResource(Microsoft::WRL::ComPtr< ID3D12Device> device, const DirectX::TexMetadata& metadata)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);//幅
	resourceDesc.Height = UINT(metadata.height);//高さ
	resourceDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);//数
	resourceDesc.DepthOrArraySize = UINT(metadata.arraySize);//奥行き　Textureの配置数
	resourceDesc.Format = metadata.format;//format
	resourceDesc.SampleDesc.Count = 1;//サンプリングカウント(1固定)
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);// textureの次元数


	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;


	//Resouceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
	return resource;
}

// ミップマップを含む画像データをテクスチャリソースへ書き込む
void DirectXCommon::UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages)
{
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel)
	{
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);

		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,			 //全領域へコピー
			img->pixels,		 //元データアドレス
			UINT(img->rowPitch), //1ラインサイズ
			UINT(img->slicePitch)//1枚サイズ
		);
		assert(SUCCEEDED(hr));
	}
}

// テクスチャファイルを読み込み、ミップマップを生成する
DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath)
{
	//テクスチャファイル // byte関連
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	ThrowIfFailed(hr, "Loading a texture");

	//ミップマップ　//拡大縮小で使う
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	ThrowIfFailed(hr, "Generating texture mipmaps");

	//ミップマップ付きのデータを返す
	return mipImages;
}

// 描画開始前にバックバッファ、RTV/DSV、描画領域を設定する
void DirectXCommon::PreDraw()
{
	// 書き込むバックバッファのインデックスの取得
	UINT backBafferIndex = swapChain->GetCurrentBackBufferIndex();

	// バリアを張る
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張るリソース。
	barrier.Transition.pResource = swapChainResources[backBafferIndex].Get();
	// 遷移前のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);

	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHadle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandles[backBafferIndex], false, &dsvHadle);
	// 指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f }; // 青っぽい色。RGBAの順
	commandList->ClearRenderTargetView(rtvHandles[backBafferIndex], clearColor, 0, nullptr);
	// 指定した震度で画面全体をクリアする
	commandList->ClearDepthStencilView(dsvHadle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	// 描画用のDescriptorHeapの設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
}

// 描画後に画面表示、GPU同期、次フレーム準備を行う
void DirectXCommon::PostDraw()
{
	HRESULT hr;
	// 書き込むバックバッファのインデックスの取得
	UINT backBafferIndex = swapChain->GetCurrentBackBufferIndex();

	// 画面に描画はすべて終わり、画面に移す
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);

	// コマンドリストの内容を確定させる。すべてのコマンドを詰んでからCloseする
	hr = commandList->Close();
	ThrowIfFailed(hr, "Closing the command list");

	// GPUにコマンドリストの実行を行わせる
	Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists->GetAddressOf());
	// GPUとosに画面王交換を行うよう通知する
	swapChain->Present(1, 0);

	// Fenceの更新
	fenceValue++;
	// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
	hr = commandQueue->Signal(fence.Get(), fenceValue);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to signal the Direct3D fence.");
	}
	// Fenceの値が指定したSignal値にたどり着いているか確認する
	// GetCompletedValueの初期値はFence作成時に渡した初期値
	if (fence->GetCompletedValue() < fenceValue) {
		// 指定したSignalにたどりつかないので、たどり着くまで待つようにイベントを設定する
		hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to set the Direct3D fence event.");
		}
		// イベントを待つ
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	UpdateFixFPS();

	// 次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	ThrowIfFailed(hr, "Resetting the command allocator");
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	ThrowIfFailed(hr, "Resetting the command list");
}

//===============
// 初期化関数群
//===============

// デバッグレイヤー、DXGIファクトリ、Direct3Dデバイスを作成する
void DirectXCommon::CreateInitialze()
{
#ifdef _DEBUG
	Microsoft::WRL::ComPtr <ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		//デバッグレイヤーを有効にする
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif

#pragma region Factoryの生成

	dxgiFactory = nullptr;

	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region アダプタの作成

	//使用するアダプタ用の変数。最初にnullptrを入れておく
	Microsoft::WRL::ComPtr < IDXGIAdapter4> useAdapter = nullptr;
	//良い順にアダプタを読む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; i++) {
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		//ソフトウェアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタの情報をログに出力。wstringのほうなので注意
			Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	//適切なアダプタが見つからなかったら起動できなくする
	assert(useAdapter != nullptr);

#pragma endregion

#pragma region Deviceの生成

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12,0" };

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {
			Logger::Log(std::format("FEatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}

	assert(device != nullptr);
	Logger::Log("Complete create D3D12Device!!!\n");

#pragma endregion

#ifdef _DEBUG
	Microsoft::WRL::ComPtr < ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//緊急時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumCategories = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		infoQueue->PushStorageFilter(&filter);


		//解放
		//infoQueue->Release();
	}
#endif
}

// コマンドキュー、コマンドアロケータ、コマンドリストを作成する
void DirectXCommon::CreateCommand()
{
	HRESULT hr;

#pragma region コマンドキュー

	//コマンドキュー生成
	commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	
	//生成できない場合
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region コマンドアロケータ

	//コマンドアロケータ生成
	commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	
	//生成できない場合
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region コマンドリスト

	//コマンドリスト生成
	commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	
	//生成できない場合
	assert(SUCCEEDED(hr));

#pragma endregion
}

// ウィンドウに表示するためのスワップチェーンを作成する
void DirectXCommon::CreateSwapChain()
{
	HRESULT hr;

	//スワップチェイン生成
	swapChainDesc.Width = WinApp::kClientWidth;
	swapChainDesc.Height = WinApp::kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast <IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

// 深度バッファを作成する
void DirectXCommon::CreateDepthBuffer()
{
	depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kClientWidth, WinApp::kClientHeight);
}

// SRV、RTV、DSV用のディスクリプタヒープを作成する
void DirectXCommon::CreateDescritorHeap()
{
	// SRV用のディスクリプタヒープの作成
	srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	// RTV用のディスクリプタヒープの生成
	rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	// DSV用のディスクリプタヒープの作成
	dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	// 各デスクリプタサイズを取得
	descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

// スワップチェーンのバックバッファにRTVを作成する
void DirectXCommon::CreateRenderTargetView()
{
	HRESULT hr;

	//SwapchainからResourceを引っ張ってくる
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));

	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	//RTV
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();


	for (uint32_t i = 0; i < 2; ++i) 
	{
		rtvHandles[0] = rtvStartHandle;

		// 2つ目のディスクリプタハンドルの取得
		rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		device->CreateRenderTargetView(swapChainResources[i].Get(), &rtvDesc, rtvHandles[i]);
	}

}

// 深度ステンシルリソースにDSVを作成する
void DirectXCommon::CreateDepthStencilView()
{
	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	// DSVHeapの先頭にDSVをつくる
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

// GPUとの同期に使うフェンスを作成する
void DirectXCommon::CreateFence()
{
	HRESULT hr;

	fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create the Direct3D fence.");
	}

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr) {
		throw std::runtime_error("Failed to create the Direct3D fence event.");
	}
}

// 描画に使用するビューポートを設定する
void DirectXCommon::CreateViewport()
{
	viewport.Width = WinApp::kClientWidth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

// 描画範囲を制限するシザー矩形を設定する
void DirectXCommon::CreateScissorRect()
{
	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;
}

// シェーダーコンパイル用のDXC関連オブジェクトを作成する
void DirectXCommon::CreateDXC()
{
	HRESULT hr;

	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	ThrowIfFailed(hr, "Creating DXC utilities");

	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	ThrowIfFailed(hr, "Creating the DXC compiler");

	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	ThrowIfFailed(hr, "Creating the DXC include handler");
}

// FPS固定処理で使う基準時間を初期化する
void DirectXCommon::CreateFixFPS()
{
	// 現在時間を記録する
	reference_ = std::chrono::steady_clock::now();
}

// 60FPSになるようにフレーム時間を調整する
void DirectXCommon::UpdateFixFPS()
{
	// 1/60秒ぴったりの時間
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	// 1/60秒よりわすかに短い時間
	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	// 現在時間を取得する
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	// 前回記録からの経過時間を取得する
	std::chrono::microseconds elapsep =
		std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	// 1/60秒（よりわずかに短い時間）経っていない場合
	if (elapsep < kMinCheckTime) {
		// 1/60秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			// 1マイク秒スリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	// 現在の時間を記録する
	reference_ = std::chrono::steady_clock::now();
}
