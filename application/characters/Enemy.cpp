#include "application/characters/Enemy.h"

#include "engine/3D/camera/Camera.h"
#include <cmath>

void Enemy::Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
	const std::shared_ptr<Model>& model, const Vector3& position, float radius) {
	// 敵ごとの座標と色は個別に持ち、GPUメッシュは全敵で共有する
	object_ = std::make_unique<Object3d>();
	object_->Initialize(object3dCommon, textureManager, model);
	object_->GetTransform().translate = position;
	object_->GetTransform().scale = {radius, radius, radius};
	object_->GetMaterialData()->color = {1.0f, 0.38f, 0.3f, 1.0f};
	radius_ = radius;
}

void Enemy::Update(const Camera& camera, float deltaTime, float rotationSpeed) {
	// デバッグエディタの回転速度を秒単位で適用する
	object_->GetTransform().rotate.y += rotationSpeed * deltaTime;
	object_->Update(camera);
}

// 3D共通パイプラインはGameSceneが設定してから呼び出す
void Enemy::Draw() const { object_->Draw(); }

// カメラ直前まで到達した時点を取り逃しとして扱う
bool Enemy::IsPassed(float cameraZ) const { return object_->GetTransform().translate.z < cameraZ + 0.8f; }

bool Enemy::IntersectsRay(const Vector3& origin, const Vector3& direction, float& distance) const {
	// レイと球の二次方程式を解き、カメラに最も近い交点を求める
	const Vector3& center = object_->GetTransform().translate;
	const Vector3 offset{origin.x - center.x, origin.y - center.y, origin.z - center.z};
	const float b = offset.x * direction.x + offset.y * direction.y + offset.z * direction.z;
	const float c = offset.x * offset.x + offset.y * offset.y + offset.z * offset.z - radius_ * radius_;
	const float discriminant = b * b - c;
	// 判別式が負の場合は射線が球へ届かない
	if (discriminant < 0.0f) { return false; }
	distance = -b - std::sqrt(discriminant);
	return distance >= 0.0f;
}
