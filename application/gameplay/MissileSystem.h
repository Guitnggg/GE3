#pragma once

#include "engine/3D/object/Object3d.h"
#include "engine/effects/particle/GPUParticleSystem.h"

#include <memory>
#include <vector>

class AssetManager;
class Camera;
class Fighter;
class Input;
class Model;
struct SceneContext;

class MissileSystem final {
  public:
	void Initialize(const SceneContext &context);
	void Reset();
	bool Update(Input &input, Fighter &player, Fighter &enemy, bool lockedOn, float deltaTime);
	void UpdateVisuals(const Camera &camera);
	void Draw(const Camera &camera, float deltaTime);

  private:
	struct Missile {
		Object3d object;
		Vector3 position{};
		Vector3 velocity{};
		bool firedByPlayer = true;
		float life = 0.0f;
	};

	void Spawn(const Fighter &owner, bool firedByPlayer);
	void EmitTrail(const Vector3 &position, const Vector3 &velocity);
	void EmitExplosion(const Vector3 &position);

	const SceneContext *context_ = nullptr;
	std::shared_ptr<Model> model_;
	std::vector<std::unique_ptr<Missile>> missiles_;
	GPUParticleSystem particles_;
	float playerCooldown_ = 0.0f;
	float enemyCooldown_ = 0.0f;
	uint32_t particleSeed_ = 1;
};
