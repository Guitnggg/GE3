#include "ImGuiManager.h"

#include <stdexcept>
#include <string>

#include "DirectXCommon.h"
#include "FrameRateController.h"
#include "Time.h"
#include "WinApp.h"
#include "engine/audio/Audio.h"
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
	Transform& spriteUvTransform,
	Audio& audio,
	uint32_t soundHandle,
	FrameRateController& frameRateController,
	Time& time) {
	// 用途別に折りたためるデバッグ操作画面を構築する
	ImGui::Begin("Debug Controls");

	// 3Dモデルと球体の表示・座標・マテリアル設定
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

	// 平行光源の色・方向・強度
	if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::ColorEdit4("Light Color", &directionalLight.color.x);
		ImGui::DragFloat3("Light Direction", &directionalLight.direction.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("Intensity", &directionalLight.intensity, 0.01f, 0.0f, 10.0f);
	}

	// スプライトの表示・座標・UV設定
	if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Show Sprite", &isSprite);
		ImGui::DragFloat2("Sprite Position", &spriteTransform.translate.x, 1.0f, 0.0f, 1000.0f);
		ImGui::DragFloat2("UV Translate", &spriteUvTransform.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UV Scale", &spriteUvTransform.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UV Rotation", &spriteUvTransform.rotate.z);
	}

	DrawAudioControls(audio, soundHandle);
	DrawFrameRateControls(frameRateController);
	DrawTimeControls(time);

	ImGui::End();
}

void ImGuiManager::DrawTimeControls(Time& time) {
	if (!ImGui::CollapsingHeader("Fixed Update", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}

	// 秒単位より理解しやすい更新Hzで編集し、Timeへは固定間隔へ変換して渡す
	float fixedUpdateRate = 1.0f / time.GetFixedDeltaTime();
	if (ImGui::SliderFloat("Fixed Update Rate", &fixedUpdateRate, 15.0f, 240.0f, "%.0f Hz")) {
		time.SetFixedDeltaTime(1.0f / fixedUpdateRate);
	}

	int maxSteps = static_cast<int>(time.GetMaxFixedStepsPerFrame());
	if (ImGui::SliderInt("Max Catch-up Steps", &maxSteps, 1, 16)) {
		time.SetMaxFixedStepsPerFrame(static_cast<uint32_t>(maxSteps));
	}

	// UI構築はTime::Updateより先なので、ここには直前の描画フレームの結果を表示する
	ImGui::Text("Previous frame steps: %u", time.GetFixedStepsThisFrame());
	ImGui::Text("Interpolation alpha: %.3f", time.GetFixedInterpolationAlpha());
	if (time.WasFixedTimeDroppedThisFrame()) {
		ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.2f, 1.0f),
			"Catch-up limit reached; excess fixed time was dropped.");
	}
}

void ImGuiManager::DrawFrameRateControls(FrameRateController& frameRateController) {
	if (!ImGui::CollapsingHeader("Frame Rate", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}

	static constexpr const char* kModeNames[] = {"VSync", "Limited", "Unlimited"};
	int modeIndex = static_cast<int>(frameRateController.GetMode());
	if (ImGui::Combo("Mode", &modeIndex, kModeNames, IM_ARRAYSIZE(kModeNames))) {
		frameRateController.SetMode(static_cast<FrameRateMode>(modeIndex));
	}

	float targetFPS = static_cast<float>(frameRateController.GetTargetFPS());
	if (ImGui::SliderFloat("Target FPS", &targetFPS, 15.0f, 500.0f, "%.0f FPS")) {
		frameRateController.SetTargetFPS(targetFPS);
	}

	static constexpr double kPresets[] = {30.0, 60.0, 120.0, 144.0, 165.0, 240.0};
	for (int index = 0; index < IM_ARRAYSIZE(kPresets); ++index) {
		if (index > 0) {
			ImGui::SameLine();
		}
		const int preset = static_cast<int>(kPresets[index]);
		const std::string label = std::to_string(preset);
		if (ImGui::SmallButton(label.c_str())) {
			frameRateController.SetTargetFPS(kPresets[index]);
		}
	}

	ImGui::Text("Current: %.1f FPS (%.2f ms)",
		frameRateController.GetCurrentFPS(),
		frameRateController.GetFrameTimeMilliseconds());
	if (frameRateController.GetMode() == FrameRateMode::VSync) {
		ImGui::TextUnformatted("Target FPS is ignored while VSync is enabled.");
	}
}

void ImGuiManager::DrawAudioControls(Audio& audio, uint32_t soundHandle) {
	// 再生が自然終了した場合はデバッグUI側のハンドルも無効化する。
	if (debugVoiceHandle_ != Audio::kInvalidVoiceHandle &&
		!debugAudioPaused_ && !audio.IsPlaying(debugVoiceHandle_)) {
		debugVoiceHandle_ = Audio::kInvalidVoiceHandle;
	}

	if (!ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}

	ImGui::TextUnformatted("audio/fanfare.wav");
	ImGui::Checkbox("Loop", &debugAudioLoop_);

	// スライダー操作は再生中のボイスへ即座に反映する
	if (ImGui::SliderFloat("Volume", &debugAudioVolume_, 0.0f, 1.0f, "%.2f") &&
		debugVoiceHandle_ != Audio::kInvalidVoiceHandle) {
		audio.SetVolume(debugVoiceHandle_, debugAudioVolume_);
	}
	if (ImGui::SliderFloat("Pitch", &debugAudioPitch_, 0.25f, 4.0f, "%.2fx", ImGuiSliderFlags_Logarithmic) &&
		debugVoiceHandle_ != Audio::kInvalidVoiceHandle) {
		audio.SetPitch(debugVoiceHandle_, debugAudioPitch_);
	}
	if (ImGui::SliderFloat("Master Volume", &debugMasterVolume_, 0.0f, 1.0f, "%.2f")) {
		audio.SetMasterVolume(debugMasterVolume_);
	}

	// 再生中なら停止してから、現在のUI設定で先頭から再生する
	if (ImGui::Button("Play / Restart")) {
		if (debugVoiceHandle_ != Audio::kInvalidVoiceHandle) {
			audio.Stop(debugVoiceHandle_);
		}
		debugVoiceHandle_ = audio.Play(
			static_cast<Audio::SoundHandle>(soundHandle),
			debugAudioLoop_, debugAudioVolume_, debugAudioPitch_);
		debugAudioPaused_ = false;
	}

	ImGui::SameLine();
	if (!debugAudioPaused_) {
		if (ImGui::Button("Pause") && debugVoiceHandle_ != Audio::kInvalidVoiceHandle) {
			audio.Pause(debugVoiceHandle_);
			debugAudioPaused_ = true;
		}
	}
	else if (ImGui::Button("Resume")) {
		audio.Resume(debugVoiceHandle_);
		debugAudioPaused_ = false;
	}

	ImGui::SameLine();
	if (ImGui::Button("Stop") && debugVoiceHandle_ != Audio::kInvalidVoiceHandle) {
		audio.Stop(debugVoiceHandle_);
		debugVoiceHandle_ = Audio::kInvalidVoiceHandle;
		debugAudioPaused_ = false;
	}

	ImGui::SameLine();
	if (ImGui::Button("Stop All")) {
		audio.StopAll();
		debugVoiceHandle_ = Audio::kInvalidVoiceHandle;
		debugAudioPaused_ = false;
	}

	const char* status = "Stopped";
	if (debugVoiceHandle_ != Audio::kInvalidVoiceHandle) {
		status = debugAudioPaused_ ? "Paused" : "Playing";
	}
	ImGui::Text("Status: %s", status);
	ImGui::TextUnformatted("Keyboard shortcut: 0 = one-shot playback");
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
