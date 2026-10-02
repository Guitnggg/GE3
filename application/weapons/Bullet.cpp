#include "application/weapons/Bullet.h"
#include "engine/math/MathUtility.h"

#include "application/collision/GameCollisionLayers.h"
#include "engine/3D/camera/Camera.h"
#include "engine/3D/object/Object3d.h"

#include <cmath>
#include <stdexcept>

Bullet::~Bullet() {
	if (collisionWorld_) { collisionWorld_->Unregister(collider_); }
}

void Bullet::Initialize(CollisionWorld* collisionWorld, Object3dCommon* object3dCommon, TextureManager* textureManager,
	const std::shared_ptr<Model>& model, const Vector3& position, const Vector3& direction) {
	if (!collisionWorld || !object3dCommon || !textureManager || !model) {
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
	collisionWorld_ = collisionWorld;
	collider_ = collisionWorld_->RegisterSphere({{position, radius_}, GameCollisionLayers::PlayerProjectile,
		GameCollisionLayers::Enemy, 0, true, [this](const CollisionEvent& event) {
			if (event.type == CollisionEventType::Enter && event.otherLayer == GameCollisionLayers::Enemy) {
				hitEnemyId_ = event.otherUserData;
			}
		}});
}

void Bullet::Update(const Camera& camera, float deltaTime) {
	if (!object_) { throw std::logic_error("Bullet is not initialized."); }
	auto& position = object_->GetTransform().translate;
	position.x += velocity_.x * deltaTime;
	position.y += velocity_.y * deltaTime;
	position.z += velocity_.z * deltaTime;
	remainingLifetime_ -= deltaTime;
	collisionWorld_->SetSphere(collider_, {position, radius_});
	object_->Update(camera);
}

void Bullet::Draw() const {
	if (!object_) { throw std::logic_error("Bullet is not initialized."); }
	object_->Draw();
}

uint64_t Bullet::ConsumeHitEnemyId() {
	const uint64_t id = hitEnemyId_;
	hitEnemyId_ = 0;
	return id;
}
