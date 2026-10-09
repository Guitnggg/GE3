#pragma once

#include <memory>

class DirectXCommon;
class CollisionWorld;
class FrameRateController;
class GPUParticlePipeline;
class ShaderCompiler;
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
class AssetManager;
class DebugOverlay;
struct EngineSettings;

/// <summary>
/// すべてのゲームで共通して使用するエンジン機能を管理する基底クラス。
/// </summary>
class Engine {
  public:
	Engine();
	virtual ~Engine();

	Engine(const Engine &) = delete;
	Engine &operator=(const Engine &) = delete;

	/// <summary>
	/// 初期化から終了までのメインループを実行する。
	/// </summary>
	void Run();

  protected:
	/// <summary>
	/// エンジン初期化後に一度だけ呼ばれるゲーム初期化処理。
	/// </summary>
	virtual void OnInitialize();

	/// <summary>
	/// 共通システム更新後に毎フレーム呼ばれるゲーム更新処理。
	/// </summary>
	virtual void OnUpdate();

	/// <summary>
	/// 一定時間間隔で呼ばれるゲームロジック・物理更新処理。
	/// </summary>
	virtual void OnFixedUpdate();

	/// <summary>
	/// 描画可能なフレーム内で呼ばれるゲーム描画処理。
	/// </summary>
	virtual void OnDraw();

	/// <summary>
	/// エンジン終了前に一度だけ呼ばれるゲーム終了処理。
	/// </summary>
	virtual void OnFinalize();

	// 派生クラスから利用するゲーム共通機能
	std::unique_ptr<WinApp> winApp_;
	std::unique_ptr<Input> input_;
	std::unique_ptr<Audio> audio_;
	std::unique_ptr<CollisionWorld> collisionWorld_;
	std::unique_ptr<DirectXCommon> dxCommon_;
	std::unique_ptr<SrvManager> srvManager_;
	std::unique_ptr<TextureManager> textureManager_;
	std::unique_ptr<ModelManager> modelManager_;
	std::unique_ptr<SpriteCommon> spriteCommon_;
	std::unique_ptr<Object3dCommon> object3dCommon_;
	std::unique_ptr<GPUParticlePipeline> gpuParticlePipeline_;
	std::unique_ptr<ShaderCompiler> shaderCompiler_;
	std::unique_ptr<Time> time_;
	std::unique_ptr<FrameRateController> frameRateController_;
	std::unique_ptr<AssetManager> assetManager_;
	std::unique_ptr<EngineSettings> engineSettings_;

#ifdef _DEBUG
	std::unique_ptr<ImGuiManager> imguiManager_;
	std::unique_ptr<DebugOverlay> debugOverlay_;
#endif

  private:
	void Initialize();
	void BeginFrame();
	void BeginDraw();
	void EndDraw();
	void Finalize();
	bool IsEndRequest();

	// Finalizeの二重実行を防ぐための初期化状態
	bool initialized_ = false;
};
