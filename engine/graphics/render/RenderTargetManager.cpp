#include "engine/graphics/render/RenderTargetManager.h"

#include "engine/core/DirectXCommon.h"
#include "engine/core/WinApp.h"
#include "engine/core/diagnostics/HResult.h"
#include "engine/core/diagnostics/Logger.h"
#include "engine/graphics/shader/ShaderCompiler.h"

#include <stdexcept>
#include <string>

namespace {
// Resource本体をTYPELESSにしておくことで、同じTextureをRTVとSRVの両方から
// sRGBカラーとして参照できる。実際の色形式は各View側で指定する。
constexpr DXGI_FORMAT kColorResourceFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS;
constexpr DXGI_FORMAT kColorViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

// World描画では既存の3Dパイプラインと同じ深度形式を使用する。
constexpr DXGI_FORMAT kDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

// RenderTextureは毎フレームこの色で初期化される。
constexpr float kWorldClearColor[4] = {0.1f, 0.25f, 0.5f, 1.0f};
} // namespace

RenderTargetManager::~RenderTargetManager() {
	Finalize();
}

void RenderTargetManager::Initialize(DirectXCommon *directXCommon,
                                     SrvManager *srvManager,
                                     ShaderCompiler *shaderCompiler) {
	// 多重初期化と、不完全な描画サービスによる初期化を早い段階で拒否する。
	if (initialized_ || directXCommon == nullptr || srvManager == nullptr || shaderCompiler == nullptr) {
		throw std::invalid_argument("RenderTargetManager requires initialized rendering services.");
	}

	directXCommon_ = directXCommon;
	srvManager_ = srvManager;
	shaderCompiler_ = shaderCompiler;

	// 作成順序には依存関係がある。
	// Composite用PSOはColor TextureのView形式に合わせて作成するため、Textureを先に準備する。
	try {
		CreateColorTextures();
		CreateDepthTexture();
		CreateCompositePipeline();

		initialized_ = true;
	} catch (...) {
		// 途中まで確保できていた場合も、SRVスロットを含めて巻き戻す。
		Finalize();
		throw;
	}
}

void RenderTargetManager::BeginWorldPass() {
	if (!initialized_) {
		throw std::logic_error("RenderTargetManager is not initialized.");
	}

	BeginColorPass(kWorldTextureIndex, true);

	auto *commandList = directXCommon_->GetCommandList();

	// RTVはカラー出力先、DSVは3D描画の前後関係を判定する深度出力先となる。
	// どちらもRenderTargetManager専用なので、SwapChain側のViewとは混在しない。
	const D3D12_CPU_DESCRIPTOR_HANDLE rtv = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
	const D3D12_CPU_DESCRIPTOR_HANDLE dsv = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtv, false, &dsv);

	// 前フレームの色と深度を残さないよう、World描画前に両方を初期値へ戻す。
	commandList->ClearRenderTargetView(rtv, kWorldClearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void RenderTargetManager::BeginPostEffectPass(uint32_t textureIndex) {
	if (!initialized_) {
		throw std::logic_error("RenderTargetManager is not initialized.");
	}

	BeginColorPass(textureIndex, false);

	D3D12_CPU_DESCRIPTOR_HANDLE rtv = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
	rtv.ptr += static_cast<SIZE_T>(rtvDescriptorSize_) * textureIndex;
	constexpr float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	directXCommon_->GetCommandList()->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void RenderTargetManager::PrepareForSampling(uint32_t textureIndex) {
	ValidateTextureIndex(textureIndex);
	if (colorStates_[textureIndex] != D3D12_RESOURCE_STATE_RENDER_TARGET) {
		throw std::logic_error("Only a completed render target can be prepared for sampling.");
	}

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = colorTextures_[textureIndex].Get();
	barrier.Transition.StateBefore = colorStates_[textureIndex];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	directXCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
	colorStates_[textureIndex] = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}

D3D12_GPU_DESCRIPTOR_HANDLE RenderTargetManager::GetTextureSrv(uint32_t textureIndex) const {
	if (!initialized_) {
		throw std::logic_error("RenderTargetManager is not initialized.");
	}

	ValidateTextureIndex(textureIndex);

	// 出力中のTextureを同じPassの入力として読むことはできない。
	if (colorStates_[textureIndex] != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
		throw std::logic_error("Render texture must be prepared for sampling before requesting its SRV.");
	}

	return srvManager_->GetGPUDescriptorHandle(colorSrvs_[textureIndex]);
}

void RenderTargetManager::CompositeToSwapChain(uint32_t sourceTextureIndex) {
	if (!initialized_) {
		throw std::logic_error("RenderTargetManager is not initialized.");
	}

	ValidateTextureIndex(sourceTextureIndex);
	if (colorStates_[sourceTextureIndex] == D3D12_RESOURCE_STATE_RENDER_TARGET) {
		PrepareForSampling(sourceTextureIndex);
	}

	auto *commandList = directXCommon_->GetCommandList();

	// ここから先の出力先は表示用バックバッファとなる。
	// この切り替え後に描かれるSprite UIとImGuiはRenderTextureを通らない。
	directXCommon_->SetSwapChainRenderTarget();

	// Composite専用のRootSignatureとPSOを設定する。
	// Root引数0には、World描画結果を参照するSRVディスクリプタを渡す。
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pipelineState_.Get());
	commandList->SetGraphicsRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(colorSrvs_[sourceTextureIndex]));

	// Vertex ShaderがSV_VertexIDから頂点を生成するため、頂点バッファの設定は不要。
	// 3頂点の大きな三角形1枚で画面全体を覆い、継ぎ目のないコピーを行う。
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0);
}

void RenderTargetManager::Finalize() noexcept {
	// SRVディスクリプタはGPU Resourceとは別にSrvManagerが管理している。
	// Resourceを破棄する前にスロットを返却し、別Textureから再利用できる状態にする。
	if (srvManager_ != nullptr) {
		for (SrvHandle &handle : colorSrvs_) {
			if (srvManager_->IsValid(handle)) {
				srvManager_->Release(handle);
			}
			handle = {};
		}
	}

	// GPUオブジェクトは参照関係の上位から順に解放する。
	// EngineCore側でWaitForGPUIdle済みなので、ここではGPU待機を行わない。
	pipelineState_.Reset();
	rootSignature_.Reset();
	dsvHeap_.Reset();
	rtvHeap_.Reset();
	depthTexture_.Reset();
	for (auto &texture : colorTextures_) {
		texture.Reset();
	}

	// 外部サービスは所有していないため、deleteせず参照だけを解除する。
	shaderCompiler_ = nullptr;
	srvManager_ = nullptr;
	directXCommon_ = nullptr;
	colorStates_.fill(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	rtvDescriptorSize_ = 0;
	initialized_ = false;
}

void RenderTargetManager::CreateColorTextures() {
	auto device = directXCommon_->GetDevice();

	// ウィンドウと等倍の2D Textureを作成する。
	// MipMapや配列要素は不要で、RTVとして使用できるフラグだけを付与する。
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = WinApp::kClientWidth;
	resourceDesc.Height = WinApp::kClientHeight;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = kColorResourceFormat;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// 毎フレームGPUが読み書きするため、CPUアクセス用HeapではなくDefault Heapへ配置する。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 最適化クリア値を実際のClearRenderTargetViewと一致させる。
	// Resource本体はTYPELESSなので、Viewで使用するsRGB形式をここに指定する。
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = kColorViewFormat;
	for (size_t channel = 0; channel < 4; ++channel) {
		clearValue.Color[channel] = kWorldClearColor[channel];
	}

	// 2枚分のRTVを同じHeapへ連続配置し、Texture番号から位置を計算する。
	rtvHeap_ = directXCommon_->CreateDescriptorHeap(
	    device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kRenderTextureCount, false);
	rtvDescriptorSize_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (uint32_t index = 0; index < kRenderTextureCount; ++index) {
		// 2枚とも最初は入力用SRV状態。各Pass開始時に必要なTextureだけをRTVへ遷移する。
		HRESULT hr = device->CreateCommittedResource(&heapProperties,
		                                             D3D12_HEAP_FLAG_NONE,
		                                             &resourceDesc,
		                                             colorStates_[index],
		                                             &clearValue,
		                                             IID_PPV_ARGS(&colorTextures_[index]));
		HResult::ThrowIfFailed(hr, "Creating a Ping-Pong render texture");

		D3D12_CPU_DESCRIPTOR_HANDLE rtv = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
		rtv.ptr += static_cast<SIZE_T>(rtvDescriptorSize_) * index;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = kColorViewFormat;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		device->CreateRenderTargetView(colorTextures_[index].Get(), &rtvDesc, rtv);

		// 各Textureを次のエフェクト、または最終Compositeから読めるようSRVも作成する。
		colorSrvs_[index] = srvManager_->Allocate();

		DirectX::TexMetadata metadata{};
		metadata.width = WinApp::kClientWidth;
		metadata.height = WinApp::kClientHeight;
		metadata.depth = 1;
		metadata.arraySize = 1;
		metadata.mipLevels = 1;
		metadata.format = kColorViewFormat;
		metadata.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
		srvManager_->CreateSRVforTexture2D(colorSrvs_[index], colorTextures_[index].Get(), metadata);
	}
}

void RenderTargetManager::CreateDepthTexture() {
	auto device = directXCommon_->GetDevice();

	// SwapChain側とは別の深度Textureを用意する。
	// Worldをオフスクリーンへ描く間、カラーと深度が同じPass内で完結するようにする。
	depthTexture_ = directXCommon_->CreateDepthStencilTextureResource(
	    device, WinApp::kClientWidth, WinApp::kClientHeight);

	// DSVはShaderから参照しないため、shader-visibleではない専用Heapで十分。
	dsvHeap_ = directXCommon_->CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = kDepthFormat;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(depthTexture_.Get(), &dsvDesc, dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}

void RenderTargetManager::CreateCompositePipeline() {
	auto device = directXCommon_->GetDevice();

	// Pixel Shaderのt0へ、Color TextureのSRVを1枚だけ渡すDescriptor Tableを定義する。
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// Compositeで必要なRoot引数はSRV Tableだけ。
	// World用のカメラ行列やライト定数は、このPassでは使用しない。
	D3D12_ROOT_PARAMETER rootParameter{};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter.DescriptorTable.NumDescriptorRanges = 1;
	rootParameter.DescriptorTable.pDescriptorRanges = &descriptorRange;

	// 1:1コピー時にも線形補間を使用し、Texture端では外側を繰り返さずClampする。
	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// 上記のSRV TableとSamplerを1つのRootSignatureへまとめる。
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pParameters = &rootParameter;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &sampler;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// RootSignature記述をGPUが扱えるバイナリへ変換する。
	// 失敗時はDX12から返された詳細メッセージもログへ残す。
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(
	    &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		const char *message = errorBlob ? static_cast<const char *>(errorBlob->GetBufferPointer())
		                                : "Unknown root signature error.";
		Logger::Log(std::string(message) + "\n");
		HResult::ThrowIfFailed(hr, "Serializing the RenderTexture composite root signature");
	}

	// シリアライズ済みバイナリから実際のRootSignatureを生成する。
	hr = device->CreateRootSignature(
	    0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	HResult::ThrowIfFailed(hr, "Creating the RenderTexture composite root signature");

	// Vertex ShaderはSV_VertexIDから全画面三角形を生成し、Pixel ShaderはTextureをそのまま返す。
	// Copy.PSは、次工程でポストエフェクトへ差し替える基準Passでもある。
	auto vertexShader = shaderCompiler_->Compile(L"resource/shaders/post/Fullscreen.VS.hlsl", L"vs_6_0");
	auto pixelShader = shaderCompiler_->Compile(L"resource/shaders/post/Copy.PS.hlsl", L"ps_6_0");

	// Compositeは常に画面全体へ上書きするため、Blend・Depth・Stencilを使用しない。
	// 出力形式はSwapChainのRTV形式と一致させる。
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
	pipelineDesc.pRootSignature = rootSignature_.Get();
	pipelineDesc.VS = {vertexShader->GetBufferPointer(), vertexShader->GetBufferSize()};
	pipelineDesc.PS = {pixelShader->GetBufferPointer(), pixelShader->GetBufferSize()};
	pipelineDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pipelineDesc.RasterizerState.DepthClipEnable = TRUE;
	pipelineDesc.DepthStencilState.DepthEnable = FALSE;
	pipelineDesc.DepthStencilState.StencilEnable = FALSE;
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.NumRenderTargets = 1;
	pipelineDesc.RTVFormats[0] = directXCommon_->GetRenderTargetFormat();
	pipelineDesc.SampleDesc.Count = 1;

	// 完成した固定描画設定をPSOとして保持し、毎フレームはBindするだけにする。
	hr = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState_));
	HResult::ThrowIfFailed(hr, "Creating the RenderTexture composite pipeline");
}

void RenderTargetManager::BeginColorPass(uint32_t textureIndex, bool useDepth) {
	ValidateTextureIndex(textureIndex);

	// Ping-Pongでは入力Textureと出力Textureを分ける。
	// すでにRTV状態なら、同じTextureへのPassを二重開始した呼び出しミスとなる。
	if (colorStates_[textureIndex] != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
		throw std::logic_error("Render texture is already being used as an output target.");
	}

	auto *commandList = directXCommon_->GetCommandList();

	// Shader入力として読んでいたTextureを、次のPassのカラー出力先へ切り替える。
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = colorTextures_[textureIndex].Get();
	barrier.Transition.StateBefore = colorStates_[textureIndex];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &barrier);
	colorStates_[textureIndex] = D3D12_RESOURCE_STATE_RENDER_TARGET;

	D3D12_CPU_DESCRIPTOR_HANDLE rtv = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
	rtv.ptr += static_cast<SIZE_T>(rtvDescriptorSize_) * textureIndex;

	// World Passだけは3D深度を使用する。全画面PostEffect PassにはDSVを設定しない。
	if (useDepth) {
		const D3D12_CPU_DESCRIPTOR_HANDLE dsv = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
		commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
	} else {
		commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
	}
}

void RenderTargetManager::ValidateTextureIndex(uint32_t textureIndex) const {
	if (textureIndex >= kRenderTextureCount) {
		throw std::out_of_range("Render texture index is outside the Ping-Pong target range.");
	}
}
