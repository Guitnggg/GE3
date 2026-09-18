#pragma once

#include <d3d12.h>
class DirectXCommon;
class WinApp;

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
	bool isInitialized_ = false;          // ImGuiが初期化済みか
};
