#pragma once

class Audio;
class AssetManager;
class CollisionWorld;
class DirectXCommon;
class FrameRateController;
class GPUParticlePipeline;
class ImGuiManager;
class Input;
class ModelManager;
class Object3dCommon;
class SpriteCommon;
class TextureManager;
class Time;
class SceneManager;

/// <summary>
/// シーンへ貸し出すエンジンサービスの非所有参照をひとまとめにする。
/// </summary>
struct SceneContext {
	SceneManager *sceneManager = nullptr;               // 安全なシーン切り替えを予約する窓口
	Audio *audio = nullptr;                             // 音声の読み込みと再生を提供する非所有参照
	AssetManager *assets = nullptr;                     // エンジン資産を統一して取得する非所有参照
	CollisionWorld *collisionWorld = nullptr;           // Colliderの登録と衝突判定を提供する非所有参照
	DirectXCommon *directXCommon = nullptr;             // DirectX 12描画基盤への非所有参照
	Input *input = nullptr;                             // キーボード・マウス・パッド入力への非所有参照
	TextureManager *textureManager = nullptr;           // テクスチャ管理機能への非所有参照
	ModelManager *modelManager = nullptr;               // 3Dモデル管理機能への非所有参照
	SpriteCommon *spriteCommon = nullptr;               // スプライト共通描画機能への非所有参照
	Object3dCommon *object3dCommon = nullptr;           // 3Dオブジェクト共通描画機能への非所有参照
	Time *time = nullptr;                               // フレーム時間とゲーム内時間への非所有参照
	FrameRateController *frameRateController = nullptr; // フレーム同期とFPS情報への非所有参照
	GPUParticlePipeline *gpuParticlePipeline = nullptr; // GPUパーティクル共通パイプラインへの非所有参照

#ifdef _DEBUG
	ImGuiManager *imguiManager = nullptr; // デバッグUI描画機能への非所有参照
#endif
};
