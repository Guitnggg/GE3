#pragma once

#include "engine/3D/camera/Camera.h"

class Fighter;

class CombatCamera final {
  public:
	void Initialize();
	void Reset(const Fighter &player);
	void Update(const Fighter &player, float deltaTime);
	[[nodiscard]] const Camera &GetCamera() const { return camera_; }

  private:
	Camera camera_;
	Vector3 position_{0.0f, 36.0f, -105.0f};
};
