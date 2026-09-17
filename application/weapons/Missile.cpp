#include "application/weapons/Missile.h"

#include "application/characters/Enemy.h"
#include "engine/3D/camera/Camera.h"
#include "engine/3D/object/Object3d.h"
#include "engine/collision/Collision.h"

#include <cmath>
#include <stdexcept>

Missile::~Missile() = default;

void Missile::Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
	const std::shared_ptr<Model>& model, const Vector3& position,
	const Vector3& initialDirection, uint64_t targetId) {
	if (!object3dCommon || !textureManager || !model || targetId == 0) {
		throw std::invalid_argument("Missile requires rendering services, a model, and a target.");
	}
	object_ = std::make_unique<Object3d>();
	object_->Initialize(object3dCommon, textureManager, model);
	object_->GetTransform().translate = position;
	previousPosition_ = position;
	object_->GetTransform().scale = {0.12f, 0.12f, 0.12f};
	const Vector3 direction = Normalize(initialDirection);
	velocity_ = {direction.x * kSpeed, direction.y * kSpeed, direction.z * kSpeed};
	targetId_ = targetId;
	remainingLifetime_ = 5.0f;
}

void Missile::Update(const Camera& camera, float deltaTime, const Enemy* target) {
	if (!object_) { throw std::logic_error("Missile is not initialized."); }
	if (target != nullptr) {
		const Vector3& position = object_->GetTransform().translate;
		const Vector3 toTarget{target->GetPosition().x - position.x,
			target->GetPosition().y - position.y, target->GetPosition().z - position.z};
		const Vector3 direction = Normalize(toTarget);
		velocity_ = {direction.x * kSpeed, direction.y * kSpeed, direction.z * kSpeed};
	}
	auto& position = object_->GetTransform().translate;
	previousPosition_ = position;
	position.x += velocity_.x * deltaTime;
	position.y += velocity_.y * deltaTime;
	position.z += velocity_.z * deltaTime;
	remainingLifetime_ -= deltaTime;
	object_->Update(camera);
}

void Missile::Draw() const {
	if (!object_) { throw std::logic_error("Missile is not initialized."); }
	object_->Draw();
}

bool Missile::Intersects(const Enemy& enemy) const {
	if (!object_) { return false; }
	const Vector3& position = object_->GetTransform().translate;
	return Collision::IntersectsSegment(previousPosition_, position,
		SphereCollider{enemy.GetPosition(), enemy.GetRadius() + 0.35f});
}
