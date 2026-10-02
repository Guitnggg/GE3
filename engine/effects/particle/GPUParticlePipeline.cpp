#include "engine/effects/particle/GPUParticlePipeline.h"

#include "engine/core/DirectXCommon.h"
#include "engine/core/diagnostics/HResult.h"
#include "engine/core/diagnostics/Logger.h"
#include "engine/graphics/shader/ShaderCompiler.h"

#include <stdexcept>
#include <string>

using Microsoft::WRL::ComPtr;

namespace {
void SerializeRootSignature(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& desc,
	ComPtr<ID3D12RootSignature>& destination, const char* context) {
	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;
	const HRESULT result = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	if (FAILED(result) && error) {
		Logger::Log(std::string(static_cast<const char*>(error->GetBufferPointer())) + "\n");
	}
	HResult::ThrowIfFailed(result, context);
	HResult::ThrowIfFailed(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
		IID_PPV_ARGS(&destination)), context);
}
}

void GPUParticlePipeline::Initialize(DirectXCommon* dxCommon, ShaderCompiler* shaderCompiler) {
	if (dxCommon_ != nullptr) { throw std::logic_error("GPUParticlePipeline is already initialized."); }
	if (dxCommon == nullptr || shaderCompiler == nullptr) { throw std::invalid_argument("GPUParticlePipeline requires rendering services."); }
	dxCommon_ = dxCommon;
	shaderCompiler_ = shaderCompiler;
	CreateRootSignatures();
	CreateComputePipelines();
	CreateGraphicsPipelines();
}

ID3D12PipelineState* GPUParticlePipeline::GetGraphicsPipeline(GPUParticleBlendMode blendMode) const {
	return blendMode == GPUParticleBlendMode::Additive ? additivePipeline_.Get() : alphaPipeline_.Get();
}

void GPUParticlePipeline::CreateRootSignatures() {
	D3D12_ROOT_PARAMETER computeParameters[4]{};
	computeParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	computeParameters[0].Descriptor.ShaderRegister = 0;
	for (uint32_t i = 1; i < 4; ++i) {
		computeParameters[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		computeParameters[i].Descriptor.ShaderRegister = i - 1;
	}
	D3D12_ROOT_SIGNATURE_DESC computeDesc{};
	computeDesc.NumParameters = _countof(computeParameters);
	computeDesc.pParameters = computeParameters;
	SerializeRootSignature(dxCommon_->GetDevice().Get(), computeDesc, computeRootSignature_,
		"Creating GPU particle compute root signature");

	D3D12_DESCRIPTOR_RANGE textureRange{};
	textureRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textureRange.NumDescriptors = 1;
	textureRange.BaseShaderRegister = 1;
	D3D12_ROOT_PARAMETER graphicsParameters[3]{};
	graphicsParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	graphicsParameters[0].Descriptor.ShaderRegister = 0;
	graphicsParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	graphicsParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	graphicsParameters[1].Descriptor.ShaderRegister = 0;
	graphicsParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	graphicsParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	graphicsParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	graphicsParameters[2].DescriptorTable.pDescriptorRanges = &textureRange;
	graphicsParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	D3D12_ROOT_SIGNATURE_DESC graphicsDesc{};
	graphicsDesc.NumParameters = _countof(graphicsParameters);
	graphicsDesc.pParameters = graphicsParameters;
	graphicsDesc.NumStaticSamplers = 1;
	graphicsDesc.pStaticSamplers = &sampler;
	SerializeRootSignature(dxCommon_->GetDevice().Get(), graphicsDesc, graphicsRootSignature_,
		"Creating GPU particle graphics root signature");
}

void GPUParticlePipeline::CreateComputePipelines() {
	struct Entry { const wchar_t* path; ComPtr<ID3D12PipelineState>* pipeline; } entries[] = {
		{L"resource/shaders/particle/ParticleInitialize.CS.hlsl", &initializePipeline_},
		{L"resource/shaders/particle/ParticleEmit.CS.hlsl", &emitPipeline_},
		{L"resource/shaders/particle/ParticleUpdate.CS.hlsl", &updatePipeline_},
	};
	for (const auto& entry : entries) {
		ComPtr<IDxcBlob> shader = shaderCompiler_->Compile(entry.path, L"cs_6_0");
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = computeRootSignature_.Get();
		desc.CS = {shader->GetBufferPointer(), shader->GetBufferSize()};
		HResult::ThrowIfFailed(dxCommon_->GetDevice()->CreateComputePipelineState(
			&desc, IID_PPV_ARGS(entry.pipeline->GetAddressOf())), "Creating GPU particle compute pipeline");
	}
}

void GPUParticlePipeline::CreateGraphicsPipelines() {
	ComPtr<IDxcBlob> vertexShader = shaderCompiler_->Compile(L"resource/shaders/particle/Particle.VS.hlsl", L"vs_6_0");
	ComPtr<IDxcBlob> pixelShader = shaderCompiler_->Compile(L"resource/shaders/particle/Particle.PS.hlsl", L"ps_6_0");
	auto create = [&](bool additive, ComPtr<ID3D12PipelineState>& destination) {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = graphicsRootSignature_.Get();
		desc.VS = {vertexShader->GetBufferPointer(), vertexShader->GetBufferSize()};
		desc.PS = {pixelShader->GetBufferPointer(), pixelShader->GetBufferSize()};
		auto& blend = desc.BlendState.RenderTarget[0];
		blend.BlendEnable = TRUE;
		blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blend.DestBlend = additive ? D3D12_BLEND_ONE : D3D12_BLEND_INV_SRC_ALPHA;
		blend.BlendOp = D3D12_BLEND_OP_ADD;
		blend.SrcBlendAlpha = D3D12_BLEND_ONE;
		blend.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		desc.DepthStencilState.DepthEnable = TRUE;
		desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = dxCommon_->GetRenderTargetFormat();
		desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.SampleDesc.Count = 1;
		HResult::ThrowIfFailed(dxCommon_->GetDevice()->CreateGraphicsPipelineState(
			&desc, IID_PPV_ARGS(&destination)), "Creating GPU particle graphics pipeline");
	};
	create(false, alphaPipeline_);
	create(true, additivePipeline_);
}
