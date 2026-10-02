#include "application/characters/Enemy.h"

#include "application/collision/GameCollisionLayers.h"
#include "engine/3D/camera/Camera.h"
#include <stdexcept>

Enemy::~Enemy() {
	if (collisionWorld_) { collisionWorld_->Unregister(collider_); }
}

void Enemy::Initialize(CollisionWorld* collisionWorld, Object3dCommon* object3dCommon, TextureManager* textureManager,
	const std::shared_ptr<Model>& model, const Vector3& position, float radius, uint64_t id) {
	if (!collisionWorld) { throw std::invalid_argument("Enemy requires CollisionWorld."); }
	// 敵ごとの座標と色は個別に持ち、GPUメッシュは全敵で共有する
	object_ = std::make_unique<Object3d>();
	object_->Initialize(object3dCommon, textureManager, model);
	object_->GetTransform().translate = position;
	object_->GetTransform().scale = {radius, radius, radius};
	object_->GetMaterialData()->color = {1.0f, 0.38f, 0.3f, 1.0f};
	radius_ = radius;
	id_ = id;
	collisionWorld_ = collisionWorld;
	collider_ = collisionWorld_->RegisterSphere({{position, radius_}, GameCollisionLayers::Enemy,
		GameCollisionLayers::PlayerProjectile, id_});
}

void Enemy::SetLockedOn(bool lockedOn) {
	object_->GetMaterialData()->color = lockedOn
		? Vector4{1.0f, 0.85f, 0.1f, 1.0f}
		: Vector4{1.0f, 0.38f, 0.3f, 1.0f};
}

void Enemy::Update(const Camera& camera, float deltaTime, float rotationSpeed) {
	// デバッグエディタの回転速度を秒単位で適用する
	object_->GetTransform().rotate.y += rotationSpeed * deltaTime;
	collisionWorld_->SetSphere(collider_, {object_->GetTransform().translate, radius_});
	object_->Update(camera);
}

// 3D共通パイプラインはGameSceneが設定してから呼び出す
void Enemy::Draw() const { object_->Draw(); }

// カメラ直前まで到達した時点を取り逃しとして扱う
bool Enemy::IsPassed(float cameraZ) const { return object_->GetTransform().translate.z < cameraZ + 0.8f; }
