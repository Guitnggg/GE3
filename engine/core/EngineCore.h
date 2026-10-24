#pragma once

#include <memory>
#include <string_view>

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
class SceneManager;
class IScene;
class TextureManager;
class Time;
class WinApp;
class Audio;
class AssetManager;
class AbstractSceneFactory;
class DebugOverlay;
class RenderTargetManager;
class PostEffectManager;
struct EngineSettings;

/// <summary>
/// すべてのゲームで共通して使用するエンジン機能を管理する基底クラス。
/// </summary>
class EngineCore {
  public:
	EngineCore();

	virtual ~EngineCore();

	EngineCore(const EngineCore &) = delete;

	EngineCore &operator=(const EngineCore &) = delete;

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
	/// ポストエフェクト対象となるゲーム固有のワールド描画処理。
	/// </summary>
	virtual void OnDrawWorld();

	/// <summary>
	/// ポストエフェクト対象外となるゲーム固有のUI描画処理。
	/// </summary>
	virtual void OnDrawUI();

	/// <summary>
	/// エンジン終了前に一度だけ呼ばれるゲーム終了処理。
	/// </summary>
	virtual void OnFinalize();

	/// <summary>
	/// 最初のシーンを登録する。OnInitializeから一度だけ呼び出す。
	/// </summary>
	void SetInitialScene(std::unique_ptr<IScene> initialScene);

	/// <summary>
	/// ゲーム固有のシーンファクトリーを登録する。
	/// </summary>
	void SetSceneFactory(std::unique_ptr<AbstractSceneFactory> sceneFactory);

	/// <summary>
	/// 登録済みファクトリーを使用して最初のシーンを設定する。
	/// </summary>
	void SetInitialScene(std::string_view initialSceneId);

	/// <summary>
	/// 実行中のシーン切り替えに使用するSceneManagerを取得する。
	/// </summary>
	[[nodiscard]] SceneManager &GetSceneManager();

	// 派生クラスがゲーム処理を組み立てるために利用する共通機能
	std::unique_ptr<WinApp> winApp_;                           // ウィンドウとWindowsメッセージの管理
	std::unique_ptr<Input> input_;                             // キーボード・マウス・ゲームパッド入力
	std::unique_ptr<Audio> audio_;                             // 音声データと再生ボイスの管理
	std::unique_ptr<CollisionWorld> collisionWorld_;           // Colliderと衝突イベントの管理
	std::unique_ptr<DirectXCommon> dxCommon_;                  // DirectX 12の描画基盤
	std::unique_ptr<SrvManager> srvManager_;                   // SRVディスクリプタの割り当て管理
	std::unique_ptr<RenderTargetManager> renderTargetManager_; // World用RenderTextureと画面合成の管理
	std::unique_ptr<PostEffectManager> postEffectManager_;     // Ping-Pong方式のEffect実行順管理
	std::unique_ptr<TextureManager> textureManager_;           // テクスチャGPUリソースの管理
	std::unique_ptr<ModelManager> modelManager_;               // 共有モデルの読み込みとキャッシュ
	std::unique_ptr<SpriteCommon> spriteCommon_;               // 2D描画の共通パイプライン
	std::unique_ptr<Object3dCommon> object3dCommon_;           // 3D描画の共通パイプライン
	std::unique_ptr<GPUParticlePipeline> gpuParticlePipeline_; // GPUパーティクルの共通パイプライン
	std::unique_ptr<ShaderCompiler> shaderCompiler_;           // HLSLシェーダーのコンパイル管理
	std::unique_ptr<Time> time_;                               // フレーム時間と固定更新時間の管理
	std::unique_ptr<FrameRateController> frameRateController_; // FPS計測とフレーム同期
	std::unique_ptr<AssetManager> assetManager_;               // 各種アセットを読み込む統一窓口
	std::unique_ptr<EngineSettings> engineSettings_;           // JSONから読み込んだエンジン設定
	std::unique_ptr<SceneManager> sceneManager_;               // シーンの所有と標準ライフサイクル管理

#ifdef _DEBUG
	std::unique_ptr<ImGuiManager> imguiManager_; // ImGuiのフレームと描画管理
	std::unique_ptr<DebugOverlay> debugOverlay_; // エンジン診断情報の表示
#endif

  private:
	/// <summary>
	/// エンジンを構成する各サブシステムを依存順に初期化する。
	/// </summary>
	void Initialize();

	/// <summary>
	/// 時間・入力・デバッグUIのフレーム処理を開始する。
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// 描画コマンドの記録を開始する。
	/// </summary>
	void BeginDraw();

	/// <summary>
	/// World描画結果をSwapChainへ合成し、UI描画を開始する。
	/// </summary>
	void BeginUIDraw();

	/// <summary>
	/// 描画コマンドをGPUへ送信してフレームを完了する。
	/// </summary>
	void EndDraw();

	/// <summary>
	/// 初期化済みのサブシステムを逆順に終了する。
	/// </summary>
	void Finalize();

	/// <summary>
	/// ウィンドウまたはアプリケーションから終了が要求されたかを返す。
	/// </summary>
	bool IsEndRequest();

	bool initialized_ = false; // 初期化完了後かつ終了処理前であることを示す状態
	bool sceneInitialized_ = false; // 最初のシーンが登録され、標準ループを実行できる状態
};
