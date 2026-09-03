#include "ImGuiManager.h"

#include <stdexcept>

#include "DirectXCommon.h"
#include "WinApp.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

ImGuiManager::~ImGuiManager() {
	Finalize();
}

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon) {
	if (isInitialized_) {
		throw std::logic_error("ImGuiManager is already initialized.");
	}
	if (winApp == nullptr || dxCommon == nullptr || dxCommon->GetDevice() == nullptr ||
		dxCommon->GetSRVDescriptorHeap() == nullptr) {
		throw std::invalid_argument("ImGuiManager requires initialized WinApp and DirectXCommon instances.");
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	if (!ImGui_ImplWin32_Init(winApp->GetHwnd())) {
		ImGui::DestroyContext();
		throw std::runtime_error("Failed to initialize the ImGui Win32 backend.");
	}

	const bool dx12Initialized = ImGui_ImplDX12_Init(
		dxCommon->GetDevice().Get(),
		dxCommon->GetSwapChainBufferCount(),
		dxCommon->GetRenderTargetFormat(),
		dxCommon->GetSRVDescriptorHeap(),
		dxCommon->GetCPUDescriptorHandleSRV(0),
		dxCommon->GetGPUDescriptorHandleSRV(0));
	if (!dx12Initialized) {
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		throw std::runtime_error("Failed to initialize the ImGui DirectX 12 backend.");
	}

	isInitialized_ = true;
}

void ImGuiManager::BeginFrame() {
	if (!isInitialized_) {
		throw std::logic_error("ImGuiManager is not initialized.");
	}
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::DrawDebugWindow(
	bool& isModel,
	bool& isSphere,
	bool& isRotate,
	bool& isSprite,
	bool& textureChange,
	Material& sphereMaterial,
	Transform& sphereTransform,
	DirectionalLight& directionalLight,
	Transform& spriteTransform,
	Transform& spriteUvTransform) {
	ImGui::Begin("Debug Controls");

	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Model", &isModel);
		ImGui::Checkbox("Sphere", &isSphere);
		ImGui::ColorEdit3("Sphere Material", &sphereMaterial.color.x);
		ImGui::DragFloat3("Sphere Position", &sphereTransform.translate.x, 0.01f, -5.0f, 5.0f);
		ImGui::Checkbox("Rotate", &isRotate);
		ImGui::DragFloat3("Sphere Rotation", &sphereTransform.rotate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("Sphere Scale", &sphereTransform.scale.x, 0.01f, 0.5f, 5.0f);
		ImGui::Checkbox("Monster Ball Texture", &textureChange);
	}

	if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::ColorEdit4("Light Color", &directionalLight.color.x);
		ImGui::DragFloat3("Light Direction", &directionalLight.direction.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("Intensity", &directionalLight.intensity, 0.01f, 0.0f, 10.0f);
	}

	if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Show Sprite", &isSprite);
		ImGui::DragFloat2("Sprite Position", &spriteTransform.translate.x, 1.0f, 0.0f, 1000.0f);
		ImGui::DragFloat2("UV Translate", &spriteUvTransform.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UV Scale", &spriteUvTransform.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UV Rotation", &spriteUvTransform.rotate.z);
	}

	ImGui::End();
}

void ImGuiManager::EndFrame() {
	if (!isInitialized_) {
		throw std::logic_error("ImGuiManager is not initialized.");
	}
	ImGui::Render();
}

void ImGuiManager::Draw(ID3D12GraphicsCommandList* commandList) {
	if (!isInitialized_) {
		throw std::logic_error("ImGuiManager is not initialized.");
	}
	if (commandList == nullptr) {
		throw std::invalid_argument("ImGuiManager requires a valid command list.");
	}
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImGuiManager::Finalize() {
	if (!isInitialized_) {
		return;
	}
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	isInitialized_ = false;
}
