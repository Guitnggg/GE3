#pragma once

#include <memory>

class DirectXCommon;
class FrameRateController;
class ImGuiManager;
class Input;
class ModelManager;
class Object3dCommon;
class SpriteCommon;
class SrvManager;
class TextureManager;
class Time;
class WinApp;
class Audio;

/// <summary>
/// すべてのゲームで共通して使用するエンジン機能を管理する基底クラス。
/// </summary>
class Framework {
public:
	Framework();
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
	std::unique_ptr<WinApp> winApp_;
	std::unique_ptr<Input> input_;
	std::unique_ptr<Audio> audio_;
	std::unique_ptr<DirectXCommon> dxCommon_;
	std::unique_ptr<SrvManager> srvManager_;
	std::unique_ptr<TextureManager> textureManager_;
	std::unique_ptr<ModelManager> modelManager_;
	std::unique_ptr<SpriteCommon> spriteCommon_;
	std::unique_ptr<Object3dCommon> object3dCommon_;
	std::unique_ptr<Time> time_;
	std::unique_ptr<FrameRateController> frameRateController_;

#ifdef _DEBUG
	std::unique_ptr<ImGuiManager> imguiManager_;
#endif

private:
	// Finalizeの二重実行を防ぐための初期化状態
	bool initialized_ = false;
};
