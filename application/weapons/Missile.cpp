#include "application/weapons/Missile.h"

#include "application/characters/Enemy.h"
#include "application/collision/GameCollisionLayers.h"
#include "engine/3D/camera/Camera.h"
#include "engine/3D/object/Object3d.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

Missile::~Missile() {
	if (collisionWorld_) { collisionWorld_->Unregister(collider_); }
}

void Missile::Initialize(CollisionWorld* collisionWorld, Object3dCommon* object3dCommon, TextureManager* textureManager,
	const std::shared_ptr<Model>& model, const Vector3& position,
	const Vector3& initialDirection, uint64_t targetId) {
	if (!collisionWorld || !object3dCommon || !textureManager || !model || targetId == 0) {
		throw std::invalid_argument("Missile requires rendering services, a model, and a target.");
	}
	object_ = std::make_unique<Object3d>();
	object_->Initialize(object3dCommon, textureManager, model);
	object_->GetTransform().translate = position;
	object_->GetTransform().scale = {0.12f, 0.12f, 0.12f};
	const Vector3 direction = Normalize(initialDirection);
	velocity_ = {direction.x * kSpeed, direction.y * kSpeed, direction.z * kSpeed};
	UpdateRotation(0.0f, true);
	targetId_ = targetId;
	remainingLifetime_ = 5.0f;
	collisionWorld_ = collisionWorld;
	collider_ = collisionWorld_->RegisterSphere({{position, kRadius}, GameCollisionLayers::PlayerProjectile,
		GameCollisionLayers::Enemy, 0, true, [this](const CollisionEvent& event) {
			if (event.type == CollisionEventType::Enter && event.otherLayer == GameCollisionLayers::Enemy) {
				hitEnemyId_ = event.otherUserData;
			}
		}});
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
	position.x += velocity_.x * deltaTime;
	position.y += velocity_.y * deltaTime;
	position.z += velocity_.z * deltaTime;
	UpdateRotation(deltaTime, false);
	remainingLifetime_ -= deltaTime;
	collisionWorld_->SetSphere(collider_, {position, kRadius});
	object_->Update(camera);
}

void Missile::UpdateRotation(float deltaTime, bool snap) {
	const Vector3 direction = Normalize(velocity_);
	if (direction.x == 0.0f && direction.y == 0.0f && direction.z == 0.0f) { return; }

	// ミサイルモデルの先端はローカル+Z方向。速度ベクトルをyaw/pitchへ変換して進行方向へ向ける。
	const float horizontalLength = std::sqrt(direction.x * direction.x + direction.z * direction.z);
	const float targetPitch = std::atan2(-direction.y, horizontalLength);
	const float targetYaw = std::atan2(direction.x, direction.z);
	auto& rotation = object_->GetTransform().rotate;
	if (snap) {
		rotation.x = targetPitch;
		rotation.y = targetYaw;
		rotation.z = 0.0f;
		return;
	}

	// 角度の±pi境界をまたいでも最短方向へ旋回させる。
	const float maxRotation = kRotationSpeed * std::max(0.0f, deltaTime);
	rotation.x = MoveTowardsAngle(rotation.x, targetPitch, maxRotation);
	rotation.y = MoveTowardsAngle(rotation.y, targetYaw, maxRotation);
}

void Missile::Draw() const {
	if (!object_) { throw std::logic_error("Missile is not initialized."); }
	object_->Draw();
}

uint64_t Missile::ConsumeHitEnemyId() {
	const uint64_t id = hitEnemyId_;
	hitEnemyId_ = 0;
	return id;
}
