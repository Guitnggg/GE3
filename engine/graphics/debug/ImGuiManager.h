#pragma once

#include <d3d12.h>
#include <cstdint>

#include "engine/math/Mymath.h"

class DirectXCommon;
class FrameRateController;
class Time;
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

	/// <summary>
	/// ImGuiのWin32・DirectX 12バックエンドを初期化する。
	/// </summary>
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon);

	/// <summary>
	///	ImGuiの新しいフレームを開始する。
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// ゲーム内の表示設定やパラメーターを操作するデバッグ画面を作成する。
	/// </summary>
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
		uint32_t soundHandle,
		FrameRateController& frameRateController,
		Time& time);

	/// <summary>
	/// ImGuiのUI構築を終了し、描画データを確定する。
	/// </summary>
	void EndFrame();

	/// <summary>
	/// 確定したImGuiの描画命令をコマンドリストへ追加する。
	/// </summary>
	void Draw(ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// ImGuiのバックエンドとコンテキストを終了する。
	/// </summary>
	void Finalize();

private:
	/// <summary>
	/// 音声の再生状態とパラメーターを操作するUI項目を描画する。
	/// </summary>
	void DrawAudioControls(Audio& audio, uint32_t soundHandle);

	/// <summary>
	/// 描画同期方法、FPS上限、実測値を操作・表示するUI項目を描画する。
	/// </summary>
	void DrawFrameRateControls(FrameRateController& frameRateController);

	/// <summary>固定更新間隔、追いつき上限、実行状況を操作・表示する。</summary>
	void DrawTimeControls(Time& time);

	bool isInitialized_ = false;          // ImGuiが初期化済みか
	uint64_t debugVoiceHandle_ = 0;       // デバッグ画面から再生した音声の識別番号
	float debugAudioVolume_ = 1.0f;       // デバッグ再生の音量
	float debugAudioPitch_ = 1.0f;        // デバッグ再生のピッチ
	float debugMasterVolume_ = 1.0f;      // 全体の音量
	bool debugAudioLoop_ = false;         // ループ再生するか
	bool debugAudioPaused_ = false;       // 一時停止中か
};
