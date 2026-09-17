#include "application/weapons/Bullet.h"

#include "application/characters/Enemy.h"
#include "engine/3D/camera/Camera.h"
#include "engine/3D/object/Object3d.h"

#include <cmath>
#include <stdexcept>

Bullet::~Bullet() = default;

void Bullet::Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
	const std::shared_ptr<Model>& model, const Vector3& position, const Vector3& direction) {
	if (!object3dCommon || !textureManager || !model) {
		throw std::invalid_argument("Bullet requires initialized rendering services and a model.");
	}
	object_ = std::make_unique<Object3d>();
	object_->Initialize(object3dCommon, textureManager, model);
	object_->GetTransform().translate = position;
	object_->GetTransform().scale = {radius_, radius_, 0.45f};
	object_->GetMaterialData()->color = {0.25f, 0.9f, 1.0f, 1.0f};
	const Vector3 normalized = Normalize(direction);
	velocity_ = {normalized.x * kSpeed, normalized.y * kSpeed, normalized.z * kSpeed};
	remainingLifetime_ = 2.0f;
}

void Bullet::Update(const Camera& camera, float deltaTime) {
	if (!object_) { throw std::logic_error("Bullet is not initialized."); }
	auto& position = object_->GetTransform().translate;
	position.x += velocity_.x * deltaTime;
	position.y += velocity_.y * deltaTime;
	position.z += velocity_.z * deltaTime;
	remainingLifetime_ -= deltaTime;
	object_->Update(camera);
}

void Bullet::Draw() const {
	if (!object_) { throw std::logic_error("Bullet is not initialized."); }
	object_->Draw();
}

bool Bullet::Intersects(const Enemy& enemy, float deltaTime) const {
	if (!object_) { return false; }
	const Vector3& position = object_->GetTransform().translate;
	const Vector3& target = enemy.GetPosition();
	const float x = target.x - position.x;
	const float y = target.y - position.y;
	const float z = target.z - position.z;
	// 高速な通常弾が敵を1フレームで通過しないよう移動距離を判定幅へ加える
	const float hitRadius = radius_ + enemy.GetRadius() + kSpeed * deltaTime;
	return x * x + y * y + z * z <= hitRadius * hitRadius;
}
