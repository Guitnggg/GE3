#pragma once

class DirectXCommon;
class ImGuiManager;
class Input;
class Object3dCommon;
class SpriteCommon;
class SrvManager;
class TextureManager;
class WinApp;
class Audio;

/// <summary>
/// すべてのゲームで共通して使用するエンジン機能を管理する基底クラス。
/// </summary>
class Framework {
public:
	Framework() = default;
	virtual ~Framework();

	Framework(const Framework&) = delete;
	Framework& operator=(const Framework&) = delete;

	/// <summary>
	/// ウィンドウやDirectXなど、ゲーム共通機能を初期化する。
	/// </summary>
	virtual void Initialize();

	/// <summary>
	/// 入力と音声の状態を毎フレーム更新する。
	/// </summary>
	virtual void Update();

	/// <summary>
	/// ゲーム固有の描画処理。派生クラスで実装する。
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// ゲーム共通機能を終了し、確保したリソースを解放する。
	/// </summary>
	virtual void Finalize();

	/// <summary>
	/// ウィンドウの終了要求を処理して返す。
	/// </summary>
	bool IsEndRequest();

protected:
	/// <summary>
	/// 1フレーム分の描画を開始する。
	/// </summary>
	void BeginDraw();

	/// <summary>
	/// ImGuiを描画し、1フレーム分の描画を完了する。
	/// </summary>
	void EndDraw();

	// 派生クラスから利用するゲーム共通機能
	WinApp* winApp_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	SpriteCommon* spriteCommon_ = nullptr;
	Object3dCommon* object3dCommon_ = nullptr;

#ifdef _DEBUG
	ImGuiManager* imguiManager_ = nullptr;
#endif

private:
	// Finalizeの二重実行を防ぐための初期化状態
	bool initialized_ = false;
};
