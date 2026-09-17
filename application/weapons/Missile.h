#pragma once

#include "engine/math/Mymath.h"

#include <cstdint>
#include <memory>

class Camera;
class Enemy;
class Model;
class Object3d;
class Object3dCommon;
class TextureManager;

/// <summary>ロック対象をIDで追跡するミサイル1発分の誘導、寿命、描画を管理する。</summary>
class Missile final {
public:
	~Missile();
	void Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
		const std::shared_ptr<Model>& model, const Vector3& position,
		const Vector3& initialDirection, uint64_t targetId);
	void Update(const Camera& camera, float deltaTime, const Enemy* target);
	void Draw() const;
	bool Intersects(const Enemy& enemy) const;
	bool IsExpired() const { return remainingLifetime_ <= 0.0f; }
	uint64_t GetTargetId() const { return targetId_; }

private:
	std::unique_ptr<Object3d> object_;
	Vector3 previousPosition_{};
	Vector3 velocity_{};
	uint64_t targetId_ = 0;
	float remainingLifetime_ = 0.0f;
	static constexpr float kSpeed = 25.0f;
};
