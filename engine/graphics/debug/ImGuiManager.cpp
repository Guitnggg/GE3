#include "ImGuiManager.h"

#include <stdexcept>
#include "engine/core/DirectXCommon.h"
#include "engine/core/WinApp.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

ImGuiManager::~ImGuiManager() {
	Finalize();
}

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon) {
	// 二重初期化や、必要なDirectXリソースが揃っていない状態を防ぐ
	if (isInitialized_) {
		throw std::logic_error("ImGuiManager is already initialized.");
	}
	if (winApp == nullptr || dxCommon == nullptr || dxCommon->GetDevice() == nullptr ||
		dxCommon->GetSRVDescriptorHeap() == nullptr) {
		throw std::invalid_argument("ImGuiManager requires initialized WinApp and DirectXCommon instances.");
	}

	// ImGui本体のコンテキストを作成し、標準のダークテーマを適用する
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// Windowsからマウス・キーボード入力を受け取るバックエンドを初期化する
	if (!ImGui_ImplWin32_Init(winApp->GetHwnd())) {
		ImGui::DestroyContext();
		throw std::runtime_error("Failed to initialize the ImGui Win32 backend.");
	}

	// SRVヒープの0番をImGuiのフォントテクスチャ用に使用する
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
	// DirectX、Win32、ImGui本体の順に新しいフレームを開始する
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
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
	// 初期化とは逆順に各バックエンドとコンテキストを終了する
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	isInitialized_ = false;
}
