#include "engine/effects/particle/GPUParticleSystem.h"
#include "engine/effects/particle/GPUParticlePipeline.h"

#include "engine/3D/camera/Camera.h"
#include "engine/core/DirectXCommon.h"
#include "engine/core/diagnostics/HResult.h"
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

}

void GPUParticleSystem::Initialize(DirectXCommon* dxCommon, GPUParticlePipeline* pipeline, TextureManager* textureManager,
	uint32_t textureHandle, uint32_t maxParticles) {
	if (!dxCommon || !pipeline || !textureManager || maxParticles == 0) {
		throw std::invalid_argument("GPUParticleSystem requires services and at least one particle.");
	}
	dxCommon_ = dxCommon;
	pipeline_ = pipeline;
	textureManager_ = textureManager;
	textureHandle_ = textureHandle;
	maxParticles_ = maxParticles;
	pendingEmits_.reserve(kMaxEmitCommandsPerFrame);
	CreateResources();
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
	commandList->SetComputeRootSignature(pipeline_->GetComputeRootSignature());
	if (needsInitialize_) { DispatchInitialize(); }
	DispatchEmitCommands();
	DispatchUpdate();
	TransitionParticleBuffer(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	particleBufferIsSrv_ = true;

	drawConstants_->viewProjection = camera.GetViewProjectionMatrix();
	const Matrix4x4& cameraWorld = camera.GetWorldMatrix();
	drawConstants_->cameraRight = {cameraWorld.m[0][0], cameraWorld.m[0][1], cameraWorld.m[0][2]};
	drawConstants_->cameraUp = {cameraWorld.m[1][0], cameraWorld.m[1][1], cameraWorld.m[1][2]};

	commandList->SetGraphicsRootSignature(pipeline_->GetGraphicsRootSignature());
	commandList->SetPipelineState(pipeline_->GetGraphicsPipeline(preset_.blendMode));
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
	commandList->SetPipelineState(pipeline_->GetInitializePipeline());
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
	commandList->SetPipelineState(pipeline_->GetEmitPipeline());
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
	commandList->SetPipelineState(pipeline_->GetUpdatePipeline());
	commandList->SetComputeRootConstantBufferView(0, simulationConstantResource_->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(1, particleResource_->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(2, freeListResource_->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(3, freeListIndexResource_->GetGPUVirtualAddress());
	commandList->Dispatch((maxParticles_ + kThreadGroupSize - 1) / kThreadGroupSize, 1, 1);
}
