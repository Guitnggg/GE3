#pragma once

class Audio;
class FrameRateController;
class ImGuiManager;
class Input;
class ModelManager;
class Object3dCommon;
class SpriteCommon;
class TextureManager;
class Time;

/// <summary>
/// シーンが利用するエンジン共通機能への参照をまとめた構造体。
/// 所有権はFrameworkが持ち、シーンはこれらを解放しない。
/// </summary>
struct SceneContext {
	Audio* audio = nullptr;
	Input* input = nullptr;
	TextureManager* textureManager = nullptr;
	ModelManager* modelManager = nullptr;
	SpriteCommon* spriteCommon = nullptr;
	Object3dCommon* object3dCommon = nullptr;
	Time* time = nullptr;
	FrameRateController* frameRateController = nullptr;
#ifdef _DEBUG
	ImGuiManager* imguiManager = nullptr;
#endif
};
