#pragma once

#include "engine/3D/object/Object3d.h"
#include <memory>

class Camera;
class Model;
class Object3dCommon;
class TextureManager;

/// <summary>1体の敵の表示、移動判定、射線判定を管理する。</summary>
class Enemy final {
public:
	void Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
		const std::shared_ptr<Model>& model, const Vector3& position, float radius);
	void Update(const Camera& camera, float deltaTime, float rotationSpeed);
	void Draw() const;
	bool IsPassed(float cameraZ) const;
	bool IntersectsRay(const Vector3& origin, const Vector3& direction, float& distance) const;

private:
	std::unique_ptr<Object3d> object_;
	float radius_ = 1.0f;
};
