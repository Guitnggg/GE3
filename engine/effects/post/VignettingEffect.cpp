#include "engine/effects/post/VignettingEffect.h"

#include "engine/core/DirectXCommon.h"
#include "engine/core/diagnostics/HResult.h"
#include "engine/core/diagnostics/Logger.h"
#include "engine/graphics/shader/ShaderCompiler.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>

void VignettingEffect::Initialize(DirectXCommon *directXCommon, ShaderCompiler *shaderCompiler) {
	// EffectはPostEffectManagerから一度だけ初期化される。
	// 不完全な描画サービスや、同じInstanceの二重登録は明示的に拒否する。
	if (initialized_ || directXCommon == nullptr || shaderCompiler == nullptr) {
		throw std::invalid_argument("VignettingEffect requires initialized rendering services.");
	}

	directXCommon_ = directXCommon;
	shaderCompiler_ = shaderCompiler;

	try {
		// PSOはRootSignatureを参照するため、必ずRootSignatureから先に作成する。
		CreateRootSignature();
		CreatePipelineState();

		initialized_ = true;
	} catch (...) {
		pipelineState_.Reset();
		rootSignature_.Reset();
		shaderCompiler_ = nullptr;
		directXCommon_ = nullptr;
		throw;
	}
}

void VignettingEffect::Draw(D3D12_GPU_DESCRIPTOR_HANDLE sourceTexture) {
	if (!initialized_) {
		throw std::logic_error("VignettingEffect is not initialized.");
	}

	auto *commandList = directXCommon_->GetCommandList();

	// Constant Bufferはフレームごとの動的Upload領域から確保する。
	// FrameContextがGPU完了後にまとめて再利用するため、Effect側で個別解放する必要はない。
	const DirectXCommon::DynamicBufferAllocation parameterBuffer =
	    directXCommon_->AllocateDynamicBuffer(sizeof(Parameters));
	std::memcpy(parameterBuffer.cpuAddress, &parameters_, sizeof(Parameters));

	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pipelineState_.Get());

	// Root引数0へ前のPassのTexture、Root引数1へVignetting調整値を渡す。
	commandList->SetGraphicsRootDescriptorTable(0, sourceTexture);
	commandList->SetGraphicsRootConstantBufferView(1, parameterBuffer.gpuAddress);

	// Fullscreen.VSはSV_VertexIDから頂点を生成するため、Vertex Bufferは使用しない。
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0);
}

void VignettingEffect::SetIntensity(float intensity) {
	// 0では画面全体が黒くなるため、調整可能な正の範囲へ制限する。
	parameters_.intensity = std::max(intensity, 0.001f);
}

void VignettingEffect::SetPower(float power) {
	// powの指数が0以下にならないよう制限し、不正値による描画崩れを防ぐ。
	parameters_.power = std::max(power, 0.001f);
}

void VignettingEffect::CreateRootSignature() {
	// t0から入力Textureを1枚読むDescriptor Tableを定義する。
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange;

	// b0にはIntensityとPowerを格納したConstant Bufferを割り当てる。
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// Texture端の外側を参照しないよう、Clamp付き線形Samplerを固定Samplerとして持つ。
	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &sampler;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(
	    &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		const char *message = errorBlob ? static_cast<const char *>(errorBlob->GetBufferPointer())
		                                : "Unknown root signature error.";
		Logger::Log(std::string(message) + "\n");
		HResult::ThrowIfFailed(hr, "Serializing the Vignetting root signature");
	}

	hr = directXCommon_->GetDevice()->CreateRootSignature(
	    0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	HResult::ThrowIfFailed(hr, "Creating the Vignetting root signature");
}

void VignettingEffect::CreatePipelineState() {
	// 全Effectで共通のFullscreen Vertex Shaderと、Vignetting固有Pixel Shaderを組み合わせる。
	auto vertexShader = shaderCompiler_->Compile(L"resource/shaders/post/Fullscreen.VS.hlsl", L"vs_6_0");
	auto pixelShader = shaderCompiler_->Compile(L"resource/shaders/post/Vignetting.PS.hlsl", L"ps_6_0");

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

	// Fullscreen PassではBlendとDepthを使わず、入力画像を加工した結果で出力全体を上書きする。
	HRESULT hr = directXCommon_->GetDevice()->CreateGraphicsPipelineState(
	    &pipelineDesc, IID_PPV_ARGS(&pipelineState_));
	HResult::ThrowIfFailed(hr, "Creating the Vignetting pipeline");
}
