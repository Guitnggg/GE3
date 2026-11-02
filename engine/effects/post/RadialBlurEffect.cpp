#include "engine/effects/post/RadialBlurEffect.h"

#include "engine/core/DirectXCommon.h"
#include "engine/core/diagnostics/HResult.h"
#include "engine/core/diagnostics/Logger.h"
#include "engine/graphics/shader/ShaderCompiler.h"

#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>

void RadialBlurEffect::Initialize(DirectXCommon *directXCommon, ShaderCompiler *shaderCompiler) {
	// 同じInstanceの二重登録と、描画サービスが揃っていない状態での初期化を拒否する。
	if (initialized_ || directXCommon == nullptr || shaderCompiler == nullptr) {
		throw std::invalid_argument("RadialBlurEffect requires initialized rendering services.");
	}

	directXCommon_ = directXCommon;
	shaderCompiler_ = shaderCompiler;

	try {
		// PSOはRootSignatureを参照するため、依存されるRootSignatureを先に生成する。
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

void RadialBlurEffect::Draw(D3D12_GPU_DESCRIPTOR_HANDLE sourceTexture) {
	if (!initialized_) {
		throw std::logic_error("RadialBlurEffect is not initialized.");
	}

	auto *commandList = directXCommon_->GetCommandList();

	// 調整値はフレーム専用Upload領域へコピーし、GPU完了後にFrameContext単位で再利用する。
	const DirectXCommon::DynamicBufferAllocation parameterBuffer =
	    directXCommon_->AllocateDynamicBuffer(sizeof(Parameters));
	std::memcpy(parameterBuffer.cpuAddress, &parameters_, sizeof(Parameters));

	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pipelineState_.Get());

	// Root引数0へ前段の結果Texture、Root引数1へRadial Blur調整値を渡す。
	commandList->SetGraphicsRootDescriptorTable(0, sourceTexture);
	commandList->SetGraphicsRootConstantBufferView(1, parameterBuffer.gpuAddress);

	// Fullscreen.VSがSV_VertexIDから三角形を生成するため、Vertex Bufferは不要。
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0);
}

void RadialBlurEffect::DrawDebugUI() {
#ifdef _DEBUG
	if (!ImGui::CollapsingHeader("Radial Blur", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}

	ImGui::Checkbox("Enabled", &enabled_);

	ImGui::BeginDisabled(!enabled_);
	ImGui::SliderFloat2("Center", &parameters_.center.x, 0.0f, 1.0f, "%.3f");
	ImGui::SliderFloat("Blur Width", &parameters_.blurWidth, 0.0f, 0.5f, "%.3f");

	// ImGuiはintを扱うため一時変数へ変換し、確定値を範囲検証付きSetterへ戻す。
	int sampleCount = static_cast<int>(parameters_.sampleCount);
	if (ImGui::SliderInt("Sample Count", &sampleCount, kMinSampleCount, kMaxSampleCount)) {
		SetSampleCount(static_cast<uint32_t>(sampleCount));
	}

	if (ImGui::Button("Reset")) {
		parameters_ = Parameters{};
	}
	ImGui::EndDisabled();

	ImGui::Spacing();
#endif
}

void RadialBlurEffect::SetCenter(const Vector2 &center) {
	// UV範囲外の中心を許可すると全画面が一方向へ流れるため、通常用途では0～1へ制限する。
	parameters_.center.x = std::clamp(center.x, 0.0f, 1.0f);
	parameters_.center.y = std::clamp(center.y, 0.0f, 1.0f);
}

void RadialBlurEffect::SetBlurWidth(float blurWidth) {
	// 負値ではSampling方向が反転するため、Effectの意味が明確な0以上に制限する。
	parameters_.blurWidth = std::max(blurWidth, 0.0f);
}

void RadialBlurEffect::SetSampleCount(uint32_t sampleCount) {
	// HLSL側の固定最大Loop数と一致させ、過剰なGPU負荷と0除算を防ぐ。
	parameters_.sampleCount = std::clamp(sampleCount, kMinSampleCount, kMaxSampleCount);
}

void RadialBlurEffect::CreateRootSignature() {
	// t0へ前段のRenderTextureを渡す、1要素のSRV Descriptor Tableを作る。
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

	// b0へCenter、Blur Width、Sample Countを格納したConstant Bufferを割り当てる。
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// 画面端を越えたSamplingは端PixelへClampし、反対側の画像が混ざることを防ぐ。
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
		HResult::ThrowIfFailed(hr, "Serializing the Radial Blur root signature");
	}

	hr = directXCommon_->GetDevice()->CreateRootSignature(
	    0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	HResult::ThrowIfFailed(hr, "Creating the Radial Blur root signature");
}

void RadialBlurEffect::CreatePipelineState() {
	// 共通Fullscreen Vertex Shaderと、Radial Blur固有Pixel Shaderを組み合わせる。
	auto vertexShader = shaderCompiler_->Compile(L"resource/shaders/post/Fullscreen.VS.hlsl", L"vs_6_0");
	auto pixelShader = shaderCompiler_->Compile(L"resource/shaders/post/RadialBlur.PS.hlsl", L"ps_6_0");

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

	// Fullscreen Passは入力を加工した結果で出力全体を上書きするため、BlendとDepthを使わない。
	HRESULT hr = directXCommon_->GetDevice()->CreateGraphicsPipelineState(
	    &pipelineDesc, IID_PPV_ARGS(&pipelineState_));
	HResult::ThrowIfFailed(hr, "Creating the Radial Blur pipeline");
}
