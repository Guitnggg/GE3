#pragma once

#include <d3d12.h>
#include <cstdint>

#include "Mymath.h"

class DirectXCommon;
class WinApp;
class Audio;

/// <summary>
/// Dear ImGuiの初期化、フレーム処理、描画、終了処理を管理するクラス
/// </summary>
class ImGuiManager {
public:
	ImGuiManager() = default;
	~ImGuiManager();

	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	void Initialize(WinApp* winApp, DirectXCommon* dxCommon);
	void BeginFrame();
	void DrawDebugWindow(
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
		uint32_t soundHandle);
	void EndFrame();
	void Draw(ID3D12GraphicsCommandList* commandList);
	void Finalize();

private:
	/// <summary>音声の再生状態とパラメーターを操作するUI項目を描画する。</summary>
	void DrawAudioControls(Audio& audio, uint32_t soundHandle);

	bool isInitialized_ = false;
	uint64_t debugVoiceHandle_ = 0;
	float debugAudioVolume_ = 1.0f;
	float debugAudioPitch_ = 1.0f;
	float debugMasterVolume_ = 1.0f;
	bool debugAudioLoop_ = false;
	bool debugAudioPaused_ = false;
};
