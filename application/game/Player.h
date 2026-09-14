#pragma once

#include "engine/2D/Sprite.h"
#include "engine/3D/camera/Camera.h"
#include "engine/math/Mymath.h"
#include <cstdint>
#include <memory>
#include <vector>

class Input;
class SpriteCommon;
class TextureManager;

/// <summary>レール上を進むプレイヤーのカメラ、照準、ライフを管理する。</summary>
class Player final {
public:
	void Initialize(SpriteCommon* spriteCommon, TextureManager* textureManager, Input* input, uint32_t texture);
	void Reset();
	bool Update(float deltaTime);
	void DrawReticle() const;
	void Damage();

	const Camera& GetCamera() const { return camera_; }
	float GetCameraZ() const { return cameraZ_; }
	uint32_t GetLives() const { return lives_; }
	bool IsDead() const { return lives_ == 0; }
	Vector3 GetShotOrigin() const;
	Vector3 GetShotDirection() const;

private:
	std::unique_ptr<Sprite> CreateReticlePart(float width, float height, uint32_t texture);
	void UpdateReticle();

	Input* input_ = nullptr;
	SpriteCommon* spriteCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	Camera camera_{};
	std::vector<std::unique_ptr<Sprite>> reticle_;
	Vector2 aim_{640.0f, 360.0f};
	float cameraZ_ = -10.5f;
	float shotFlashTimer_ = 0.0f;
	uint32_t lives_ = 3;
};
