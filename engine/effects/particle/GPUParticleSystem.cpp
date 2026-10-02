#include "engine/effects/particle/GPUParticleSystem.h"

#include "engine/3D/camera/Camera.h"
#include "engine/core/DirectXCommon.h"
#include "engine/core/diagnostics/HResult.h"
#include "engine/core/diagnostics/Logger.h"
#include "engine/graphics/resource/TextureManager.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using Microsoft::WRL::ComPtr;

namespace {
ComPtr<ID3D12Resource> CreateUavBuffer(ID3D12Device* device, uint64_t size) {
	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_DEFAULT;
	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	ComPtr<ID3D12Resource> resource;
	// D3D12ではバッファの生成時状態は実質COMMON固定。使用直前に明示的にUAVへ遷移する。
	HResult::ThrowIfFailed(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource)), "Creating a GPU particle UAV buffer");
	return resource;
}

void SerializeRootSignature(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& desc,
	ComPtr<ID3D12RootSignature>& destination, const char* context) {
	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;
	const HRESULT result = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	if (FAILED(result) && error) { Logger::Log(std::string(static_cast<const char*>(error->GetBufferPointer())) + "\n"); }
	HResult::ThrowIfFailed(result, context);
	HResult::ThrowIfFailed(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
		IID_PPV_ARGS(&destination)), context);
}
}

void GPUParticleSystem::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager,
	uint32_t textureHandle, uint32_t maxParticles) {
	if (!dxCommon || !textureManager || maxParticles == 0) {
		throw std::invalid_argument("GPUParticleSystem requires services and at least one particle.");
	}
	dxCommon_ = dxCommon;
	textureManager_ = textureManager;
	textureHandle_ = textureHandle;
	maxParticles_ = maxParticles;
	pendingEmits_.reserve(kMaxEmitCommandsPerFrame);
	CreateResources();
	CreateRootSignatures();
	CreateComputePipeline();
	CreateGraphicsPipeline();
}

void GPUParticleSystem::SetPreset(const GPUParticlePreset& preset) {
	if (!std::isfinite(preset.minLifetime) || !std::isfinite(preset.maxLifetime) ||
		preset.minLifetime <= 0.0f || preset.maxLifetime < preset.minLifetime ||
		!std::isfinite(preset.drag) || preset.drag < 0.0f) {
		throw std::invalid_argument("Invalid GPU particle preset.");
	}
	preset_ = preset;
}

void GPUParticleSystem::Emit(const GPUParticleEmitData& emitData) {
	if (!dxCommon_) { throw std::logic_error("GPUParticleSystem is not initialized."); }
	if (emitData.count == 0) { return; }
	if (pendingEmits_.size() >= kMaxEmitCommandsPerFrame) {
		throw std::runtime_error("Too many GPU particle emit commands in one frame.");
	}
	pendingEmits_.push_back(emitData);
}

void GPUParticleSystem::Update(float deltaTime) {
	deltaTime_ = std::isfinite(deltaTime) ? std::max(0.0f, deltaTime) : 0.0f;
}

void GPUParticleSystem::Draw(const Camera& camera) {
	if (!dxCommon_) { throw std::logic_error("GPUParticleSystem is not initialized."); }
	auto* commandList = dxCommon_->GetCommandList();
	if (simulationBuffersAreCommon_) {
		ID3D12Resource* resources[] = {particleResource_.Get(), freeListResource_.Get(), freeListIndexResource_.Get()};
		D3D12_RESOURCE_BARRIER barriers[_countof(resources)]{};
		for (uint32_t i = 0; i < _countof(resources); ++i) {
			barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barriers[i].Transition.pResource = resources[i];
			barriers[i].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barriers[i].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			barriers[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		}
		commandList->ResourceBarrier(_countof(barriers), barriers);
		simulationBuffersAreCommon_ = false;
	}
	if (particleBufferIsSrv_) {
		TransitionParticleBuffer(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		particleBufferIsSrv_ = false;
	}
	commandList->SetComputeRootSignature(computeRootSignature_.Get());
	if (needsInitialize_) { DispatchInitialize(); }
	DispatchEmitCommands();
	DispatchUpdate();
	TransitionParticleBuffer(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	particleBufferIsSrv_ = true;

	drawConstants_->viewProjection = camera.GetViewProjectionMatrix();
	const Matrix4x4& cameraWorld = camera.GetWorldMatrix();
	drawConstants_->cameraRight = {cameraWorld.m[0][0], cameraWorld.m[0][1], cameraWorld.m[0][2]};
	drawConstants_->cameraUp = {cameraWorld.m[1][0], cameraWorld.m[1][1], cameraWorld.m[1][2]};

	commandList->SetGraphicsRootSignature(graphicsRootSignature_.Get());
	commandList->SetPipelineState(preset_.blendMode == GPUParticleBlendMode::Additive ? additivePipeline_.Get() : alphaPipeline_.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, drawConstantResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootShaderResourceView(1, particleResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(textureHandle_));
	commandList->DrawInstanced(6, maxParticles_, 0, 0);
}

void GPUParticleSystem::Reset() {
	pendingEmits_.clear();
	needsInitialize_ = true;
}

void GPUParticleSystem::CreateResources() {
	auto device = dxCommon_->GetDevice();
	particleResource_ = CreateUavBuffer(device.Get(), sizeof(Particle) * maxParticles_);
	freeListResource_ = CreateUavBuffer(device.Get(), sizeof(uint32_t) * maxParticles_);
	freeListIndexResource_ = CreateUavBuffer(device.Get(), sizeof(int32_t));
	simulationConstantResource_ = dxCommon_->CreateBufferResource(sizeof(SimulationConstants));
	emitConstantResource_ = dxCommon_->CreateBufferResource(sizeof(EmitConstants) * kMaxEmitCommandsPerFrame);
	drawConstantResource_ = dxCommon_->CreateBufferResource(sizeof(DrawConstants));
	HResult::ThrowIfFailed(simulationConstantResource_->Map(0, nullptr, reinterpret_cast<void**>(&simulationConstants_)), "Mapping particle simulation constants");
	HResult::ThrowIfFailed(emitConstantResource_->Map(0, nullptr, reinterpret_cast<void**>(&emitConstants_)), "Mapping particle emit constants");
	HResult::ThrowIfFailed(drawConstantResource_->Map(0, nullptr, reinterpret_cast<void**>(&drawConstants_)), "Mapping particle draw constants");
}

void GPUParticleSystem::CreateRootSignatures() {
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
	SerializeRootSignature(dxCommon_->GetDevice().Get(), computeDesc, computeRootSignature_, "Creating GPU particle compute root signature");

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
	SerializeRootSignature(dxCommon_->GetDevice().Get(), graphicsDesc, graphicsRootSignature_, "Creating GPU particle graphics root signature");
}

void GPUParticleSystem::CreateComputePipeline() {
	struct ShaderAndPipeline { const wchar_t* path; ComPtr<ID3D12PipelineState>* pipeline; } entries[] = {
		{L"resource/shaders/particle/ParticleInitialize.CS.hlsl", &initializePipeline_},
		{L"resource/shaders/particle/ParticleEmit.CS.hlsl", &emitPipeline_},
		{L"resource/shaders/particle/ParticleUpdate.CS.hlsl", &updatePipeline_},
	};
	for (const auto& entry : entries) {
		ComPtr<IDxcBlob> shader = dxCommon_->CompileShader(entry.path, L"cs_6_0");
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = computeRootSignature_.Get();
		desc.CS = {shader->GetBufferPointer(), shader->GetBufferSize()};
		HResult::ThrowIfFailed(dxCommon_->GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(entry.pipeline->GetAddressOf())),
			"Creating GPU particle compute pipeline");
	}
}

void GPUParticleSystem::CreateGraphicsPipeline() {
	ComPtr<IDxcBlob> vertexShader = dxCommon_->CompileShader(L"resource/shaders/particle/Particle.VS.hlsl", L"vs_6_0");
	ComPtr<IDxcBlob> pixelShader = dxCommon_->CompileShader(L"resource/shaders/particle/Particle.PS.hlsl", L"ps_6_0");
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
		HResult::ThrowIfFailed(dxCommon_->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&destination)),
			"Creating GPU particle graphics pipeline");
	};
	create(false, alphaPipeline_);
	create(true, additivePipeline_);
}

void GPUParticleSystem::TransitionParticleBuffer(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = particleResource_.Get();
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
}

void GPUParticleSystem::DispatchInitialize() {
	auto* commandList = dxCommon_->GetCommandList();
	simulationConstants_->maxParticles = maxParticles_;
	commandList->SetPipelineState(initializePipeline_.Get());
	commandList->SetComputeRootConstantBufferView(0, simulationConstantResource_->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(1, particleResource_->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(2, freeListResource_->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(3, freeListIndexResource_->GetGPUVirtualAddress());
	commandList->Dispatch((maxParticles_ + kThreadGroupSize - 1) / kThreadGroupSize, 1, 1);
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	// null指定のグローバルUAVバリアで、3つの共有バッファすべての初期化完了を保証する。
	barrier.UAV.pResource = nullptr;
	commandList->ResourceBarrier(1, &barrier);
	needsInitialize_ = false;
}

void GPUParticleSystem::DispatchEmitCommands() {
	auto* commandList = dxCommon_->GetCommandList();
	commandList->SetPipelineState(emitPipeline_.Get());
	for (size_t i = 0; i < pendingEmits_.size(); ++i) {
		const auto& source = pendingEmits_[i];
		auto& target = emitConstants_[i];
		target.position = source.position;
		target.count = std::min(source.count, maxParticles_);
		target.minVelocity = source.minVelocity;
		target.maxVelocity = source.maxVelocity;
		target.positionSpread = source.positionSpread;
		target.seed = source.seed;
		target.minLifetime = preset_.minLifetime;
		target.maxLifetime = preset_.maxLifetime;
		target.startColor = preset_.startColor;
		target.endColor = preset_.endColor;
		target.startSize = preset_.startSize;
		target.endSize = preset_.endSize;
		commandList->SetComputeRootConstantBufferView(0, emitConstantResource_->GetGPUVirtualAddress() + sizeof(EmitConstants) * i);
		commandList->SetComputeRootUnorderedAccessView(1, particleResource_->GetGPUVirtualAddress());
		commandList->SetComputeRootUnorderedAccessView(2, freeListResource_->GetGPUVirtualAddress());
		commandList->SetComputeRootUnorderedAccessView(3, freeListIndexResource_->GetGPUVirtualAddress());
		commandList->Dispatch((target.count + kThreadGroupSize - 1) / kThreadGroupSize, 1, 1);
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = nullptr;
		commandList->ResourceBarrier(1, &barrier);
	}
	pendingEmits_.clear();
}

void GPUParticleSystem::DispatchUpdate() {
	auto* commandList = dxCommon_->GetCommandList();
	simulationConstants_->deltaTime = deltaTime_;
	simulationConstants_->maxParticles = maxParticles_;
	simulationConstants_->acceleration = preset_.acceleration;
	simulationConstants_->drag = preset_.drag;
	commandList->SetPipelineState(updatePipeline_.Get());
	commandList->SetComputeRootConstantBufferView(0, simulationConstantResource_->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(1, particleResource_->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(2, freeListResource_->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(3, freeListIndexResource_->GetGPUVirtualAddress());
	commandList->Dispatch((maxParticles_ + kThreadGroupSize - 1) / kThreadGroupSize, 1, 1);
}
