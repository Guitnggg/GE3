#include "application/characters/Enemy.h"

#include "application/collision/GameCollisionLayers.h"
#include "engine/2D/Sprite.h"
#include "engine/3D/camera/Camera.h"
#include "engine/core/WinApp.h"
#include <cmath>
#include <stdexcept>

namespace {
bool WorldToScreen(const Vector3 &worldPosition, const Camera &camera, Vector2 &screenPosition) {
	const Matrix4x4 &matrix = camera.GetViewProjectionMatrix();
	const float clipX = worldPosition.x * matrix.m[0][0] + worldPosition.y * matrix.m[1][0] +
	                    worldPosition.z * matrix.m[2][0] + matrix.m[3][0];
	const float clipY = worldPosition.x * matrix.m[0][1] + worldPosition.y * matrix.m[1][1] +
	                    worldPosition.z * matrix.m[2][1] + matrix.m[3][1];
	const float clipZ = worldPosition.x * matrix.m[0][2] + worldPosition.y * matrix.m[1][2] +
	                    worldPosition.z * matrix.m[2][2] + matrix.m[3][2];
	const float clipW = worldPosition.x * matrix.m[0][3] + worldPosition.y * matrix.m[1][3] +
	                    worldPosition.z * matrix.m[2][3] + matrix.m[3][3];
	if (clipW <= 0.0f) {
		return false;
	}
	const float ndcX = clipX / clipW;
	const float ndcY = clipY / clipW;
	const float ndcZ = clipZ / clipW;
	if (!std::isfinite(ndcX) || !std::isfinite(ndcY) || ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f ||
	    ndcZ < 0.0f || ndcZ > 1.0f) {
		return false;
	}
	screenPosition = {(ndcX + 1.0f) * 0.5f * WinApp::kClientWidth, (1.0f - ndcY) * 0.5f * WinApp::kClientHeight};
	return true;
}
} // namespace

Enemy::~Enemy() {
	if (collisionWorld_) {
		collisionWorld_->Unregister(collider_);
	}
}

void Enemy::Initialize(CollisionWorld *collisionWorld,
                       SpriteCommon *spriteCommon,
                       Object3dCommon *object3dCommon,
                       TextureManager *textureManager,
                       const std::shared_ptr<Model> &model,
                       uint32_t lockOnTexture,
                       const Vector3 &position,
                       float radius,
                       uint64_t id) {
	if (!collisionWorld || !spriteCommon) {
		throw std::invalid_argument("Enemy requires CollisionWorld and SpriteCommon.");
	}
	// 敵ごとの座標と色は個別に持ち、GPUメッシュは全敵で共有する
	object_ = std::make_unique<Object3d>();
	object_->Initialize(object3dCommon, textureManager, model);
	object_->GetTransform().translate = position;
	object_->GetTransform().scale = {radius, radius, radius};
	object_->GetMaterialData()->color = {1.0f, 0.38f, 0.3f, 1.0f};
	lockOnMarker_ = std::make_unique<Sprite>();
	lockOnMarker_->Initialize(spriteCommon, textureManager, lockOnTexture);
	lockOnMarker_->SetSize({72.0f, 72.0f});
	lockOnMarker_->SetAnchorPoint({0.5f, 0.5f});
	radius_ = radius;
	id_ = id;
	collisionWorld_ = collisionWorld;
	collider_ = collisionWorld_->RegisterSphere(
	    {{position, radius_}, GameCollisionLayers::Enemy, GameCollisionLayers::PlayerProjectile, id_});
}

void Enemy::SetLockedOn(bool lockedOn) {
	lockedOn_ = lockedOn;
	object_->GetMaterialData()->color = lockedOn ? Vector4{1.0f, 0.85f, 0.1f, 1.0f} : Vector4{1.0f, 0.38f, 0.3f, 1.0f};
}

void Enemy::Update(const Camera &camera, float deltaTime, float rotationSpeed) {
	// デバッグエディタの回転速度を秒単位で適用する
	object_->GetTransform().rotate.y += rotationSpeed * deltaTime;
	collisionWorld_->SetSphere(collider_, {object_->GetTransform().translate, radius_});
	object_->Update(camera);
	lockOnMarkerVisible_ = false;
	if (lockedOn_) {
		Vector2 screenPosition{};
		if (WorldToScreen(object_->GetTransform().translate, camera, screenPosition)) {
			lockOnMarker_->GetTransform().translate = {screenPosition.x, screenPosition.y, 0.0f};
			lockOnMarker_->Update(WinApp::kClientWidth, WinApp::kClientHeight);
			lockOnMarkerVisible_ = true;
		}
	}
}

// 3D共通パイプラインはGameSceneが設定してから呼び出す
void Enemy::Draw() const {
	object_->Draw();
}

void Enemy::DrawLockOnMarker() const {
	if (lockedOn_ && lockOnMarkerVisible_) {
		lockOnMarker_->Draw();
	}
}

// カメラ直前まで到達した時点を取り逃しとして扱う
bool Enemy::IsPassed(float cameraZ) const {
	return object_->GetTransform().translate.z < cameraZ + 0.8f;
}
