#include "Object3dCommon.h"

#include <stdexcept>

#include "engine/core/diagnostics/HResult.h"
#include "engine/core/diagnostics/Logger.h"
#include "engine/graphics/shader/ShaderCompiler.h"

// 3D描画共通処理を初期化する
void Object3dCommon::Initialize(DirectXCommon *directXCommon, ShaderCompiler *shaderCompiler) {
	// GPUデバイスとコマンドリストを提供する共通処理を検証して保持する
	if (directXCommon == nullptr || shaderCompiler == nullptr) {
		throw std::invalid_argument("Object3dCommon requires rendering services.");
	}
	dxCommon_ = directXCommon;
	shaderCompiler_ = shaderCompiler;
	CreateGraphicsPipeline();
}

// 3D描画で共通して使うパイプライン設定をコマンドリストへ設定する
void Object3dCommon::CommonDrawSetting() {
	// 後続のObject3dが共有するルートシグネチャ、PSO、プリミティブ形式を設定する
	auto *commandList = dxCommon_->GetCommandList();
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3dCommon::SetBlendMode(BlendMode blendMode) {
	if (blendMode >= BlendMode::Count) {
		throw std::invalid_argument("Invalid 3D blend mode.");
	}
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineStates_[static_cast<size_t>(blendMode)].Get());
}

// 3D描画用のルートシグネチャを作成する
void Object3dCommon::CreateRootSignature() {
	// 入力アセンブラーを使用できる3D描画用ルートシグネチャを定義する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// ピクセルシェーダーからテクスチャ1枚を参照するSRVテーブルを設定する
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// マテリアル、座標変換、テクスチャ、ライトの順にルート引数を配置する
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// UV範囲外を繰り返し、線形補間する静的サンプラーを設定する
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// 定義をバイナリ化し、失敗時は詳細をログへ残す
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	hr = D3D12SerializeRootSignature(
	    &descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		const char *message =
		    errorBlob ? reinterpret_cast<char *>(errorBlob->GetBufferPointer()) : "Unknown root signature error.";
		Logger::Log(std::string(message) + "\n");
		HResult::ThrowIfFailed(hr, "Serializing the 3D root signature");
	}

	// シリアライズ済みデータからGPUルートシグネチャを生成する
	hr = dxCommon_->GetDevice()->CreateRootSignature(
	    0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	HResult::ThrowIfFailed(hr, "Creating the 3D root signature");
}

// 3D描画用のグラフィックスパイプラインを作成する
void Object3dCommon::CreateGraphicsPipeline() {
	// PSOが参照するルートシグネチャを先に生成する
	CreateRootSignature();

	// 頂点バッファ内の位置、UV、法線のレイアウトを定義する
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// 背面カリングを行う基本描画状態を設定する
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// 3D描画で使用する頂点・ピクセルシェーダーをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob =
	    shaderCompiler_->Compile(L"resource/shaders/Object3d.VS.hlsl", L"vs_6_0");
	if (vertexShaderBlob == nullptr) {
		throw std::runtime_error("3D vertex shader compilation returned no output.");
	}

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob =
	    shaderCompiler_->Compile(L"resource/shaders/Object3d.PS.hlsl", L"ps_6_0");
	if (pixelShaderBlob == nullptr) {
		throw std::runtime_error("3D pixel shader compilation returned no output.");
	}

	// これまでの設定を1つのグラフィックスPSO記述へまとめる
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = {vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()};
	graphicsPipelineStateDesc.PS = {pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()};
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 手前の面だけを残せるよう、深度テストと深度書き込みを有効にする
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 不透明、通常アルファ、加算の各描画状態をキャッシュする
	for (size_t index = 0; index < static_cast<size_t>(BlendMode::Count); ++index) {
		const BlendMode mode = static_cast<BlendMode>(index);
		D3D12_BLEND_DESC blendDesc{};
		auto &target = blendDesc.RenderTarget[0];
		target.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		if (mode != BlendMode::Opaque) {
			target.BlendEnable = true;
			target.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			target.DestBlend = mode == BlendMode::Alpha ? D3D12_BLEND_INV_SRC_ALPHA : D3D12_BLEND_ONE;
			target.BlendOp = D3D12_BLEND_OP_ADD;
			target.SrcBlendAlpha = D3D12_BLEND_ONE;
			target.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
			target.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		}
		graphicsPipelineStateDesc.BlendState = blendDesc;
		depthStencilDesc.DepthWriteMask =
		    mode == BlendMode::Opaque ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;

		const HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(
		    &graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineStates_[index]));
		HResult::ThrowIfFailed(hr, "Creating a 3D graphics pipeline");
	}
}
