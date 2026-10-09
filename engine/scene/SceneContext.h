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

/// <summary>
/// シーンへ貸し出すエンジンサービスの非所有参照をひとまとめにする。
/// </summary>
struct SceneContext {
	Audio *audio = nullptr;
	AssetManager *assets = nullptr;
	CollisionWorld *collisionWorld = nullptr;
	DirectXCommon *directXCommon = nullptr;
	Input *input = nullptr;
	TextureManager *textureManager = nullptr;
	ModelManager *modelManager = nullptr;
	SpriteCommon *spriteCommon = nullptr;
	Object3dCommon *object3dCommon = nullptr;
	Time *time = nullptr;
	FrameRateController *frameRateController = nullptr;
	GPUParticlePipeline *gpuParticlePipeline = nullptr;

#ifdef _DEBUG
	ImGuiManager *imguiManager = nullptr;
#endif
};
