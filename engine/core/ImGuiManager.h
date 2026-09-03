#pragma once

#include <d3d12.h>

#include "Mymath.h"

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
		Transform& spriteUvTransform);
	void EndFrame();
	void Draw(ID3D12GraphicsCommandList* commandList);
	void Finalize();

private:
	bool isInitialized_ = false;
};
