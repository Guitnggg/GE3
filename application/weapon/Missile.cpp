#include "application/weapon/Missile.h"

#include "engine/3D/camera/Camera.h"
#include "engine/math/SpatialMath.h"

#include <algorithm>
#include <cmath>

// 発射時のモデル、座標、速度、所属チームを設定する
void Missile::Initialize(Object3dCommon *object3dCommon, TextureManager *textureManager,
                         const std::shared_ptr<Model> &model, const Vector3 &position,
                         const Vector3 &velocity, Team ownerTeam) {
	object_.Initialize(object3dCommon, textureManager, model);
	object_.GetTransform().scale = {0.24f, 0.24f, 0.24f};
	position_ = position;
	velocity_ = velocity;
	ownerTeam_ = ownerTeam;
}

// 速度を積分して位置と寿命を進める
void Missile::Advance(float deltaTime) {
	life_ += deltaTime;
	position_ = SpatialMath::Add(position_, SpatialMath::Scale(velocity_, deltaTime));
}

// 進行方向からモデルのピッチ角とヨー角を計算する
void Missile::UpdateVisual(const Camera &camera) {
	const Vector3 direction = SpatialMath::NormalizeOr(velocity_);
	auto &transform = object_.GetTransform();
	transform.translate = position_;
	transform.rotate = {-std::asin(std::clamp(direction.y, -1.0f, 1.0f)),
	                    std::atan2(direction.x, direction.z), 0.0f};
	object_.Update(camera);
}

// ミサイルモデルを描画する
void Missile::Draw() const {
	object_.Draw();
}
