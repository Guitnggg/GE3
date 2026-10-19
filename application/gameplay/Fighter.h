#pragma once

#include "engine/3D/object/Object3d.h"

#include <memory>

class Camera;
class Input;
class Model;
class Object3dCommon;
class TextureManager;

class Fighter final {
  public:
	void Initialize(Object3dCommon *object3dCommon, TextureManager *textureManager,
	                const std::shared_ptr<Model> &model, bool enemy);
	void Reset(const Vector3 &position, float yaw, float speed);
	void UpdatePlayer(Input &input, float deltaTime);
	void UpdateEnemy(const Fighter &target, float deltaTime);
	void UpdateVisual(const Camera &camera);
	void Draw() const;
	void Damage();

	[[nodiscard]] const Vector3 &GetPosition() const { return position_; }
	[[nodiscard]] float GetYaw() const { return yaw_; }
	[[nodiscard]] float GetPitch() const { return pitch_; }
	[[nodiscard]] float GetRoll() const { return roll_; }
	[[nodiscard]] float GetSpeed() const { return speed_; }
	[[nodiscard]] int GetHealth() const { return health_; }
	[[nodiscard]] bool IsAlive() const { return health_ > 0; }
	[[nodiscard]] Vector3 GetForward() const;

  private:
	Object3d object_;
	Vector3 position_{};
	float yaw_ = 0.0f;
	float pitch_ = 0.0f;
	float roll_ = 0.0f;
	float speed_ = 55.0f;
	int health_ = 3;
};
