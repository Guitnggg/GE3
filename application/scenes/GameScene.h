#pragma once

#include "engine/audio/Audio.h"
#include "engine/scene/IScene.h"

#include <cstdint>
#include <memory>

class Camera;
class Object3d;
class Sprite;

/// <summary>
/// 現在のサンプルゲーム内容を管理するシーン。
/// </summary>
class GameScene final : public IScene {
public:
	GameScene();
	~GameScene() override;

	void Initialize(const SceneContext& context) override;
	void Update() override;
	void FixedUpdate() override;
	void Draw() override;
	void Finalize() override;

private:
	SceneContext context_{};
	std::unique_ptr<Sprite> sprite_;
	std::unique_ptr<Object3d> object3d_;
	std::unique_ptr<Object3d> sphere_;
	std::unique_ptr<Camera> camera_;

	Audio::SoundHandle fanfareSound_{};
	uint32_t uvCheckerTexture_ = 0;
	uint32_t monsterBallTexture_ = 0;
	static constexpr uint32_t kSphereSubdivisions = 16;

	bool textureChange_ = true;
	bool isRotate_ = false;
	bool isModel_ = false;
	bool isSphere_ = true;
	bool isSprite_ = false;
	bool initialized_ = false;
};
