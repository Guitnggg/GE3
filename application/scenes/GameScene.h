#pragma once

#include "engine/2D/Sprite.h"
#include "engine/3D/camera/Camera.h"
#include "engine/3D/object/Object3d.h"
#include "engine/math/Mymath.h"
#include "engine/scene/IScene.h"
#include <cstdint>
#include <memory>
#include <vector>

class Model;

/// <summary>自動でコースを進む3Dレールシューティング。</summary>
class GameScene final : public IScene {
public:
	~GameScene() override;
	void Initialize(const SceneContext& context) override;
	void Update() override;
	void FixedUpdate() override;
	void Draw() override;
	void Finalize() override;

private:
	struct Target {
		std::unique_ptr<Object3d> object;
		float radius = 1.0f;
	};

	std::unique_ptr<Object3d> CreateObject(float x, float y, float z, float scale);
	std::unique_ptr<Sprite> CreateReticlePart(float width, float height);
	void ResetGame();
	void SpawnTarget();
	void Shoot();
	void UpdateRail(float deltaTime);
	void UpdateObjects();
	static bool RayHitsSphere(const Vector3& origin, const Vector3& direction, const Vector3& center,
		float radius, float& distance);

	SceneContext context_{};
	std::shared_ptr<Model> sphereModel_;
	std::unique_ptr<Camera> camera_;
	std::vector<Target> targets_;
	std::vector<std::unique_ptr<Object3d>> railMarkers_;
	std::vector<std::unique_ptr<Sprite>> reticle_;

	uint32_t texture_ = 0;
	uint32_t score_ = 0;
	uint32_t lives_ = 3;
	uint32_t spawnSequence_ = 0;
	float cameraZ_ = -10.5f;
	float targetSpawnTimer_ = 0.0f;
	float shotFlashTimer_ = 0.0f;
	Vector2 aim_{640.0f, 360.0f};
	bool gameOver_ = false;
	bool initialized_ = false;
};
