#include "application/actor/Fighter.h"

#include "engine/3D/camera/Camera.h"
#include "engine/3D/model/Model.h"
#include "engine/math/SpatialMath.h"

#include <algorithm>
#include <numbers>

using namespace SpatialMath;

//===============
// 初期化・リセット
//===============

// 機体で共有する3Dモデルと戦闘上の所属を設定する
void Fighter::Initialize(Object3dCommon *object3dCommon, TextureManager *textureManager,
                         const std::shared_ptr<Model> &model, Team team, const Vector4 &color) {
	object_.Initialize(object3dCommon, textureManager, model);
	object_.GetMaterialData()->color = color;
	team_ = team;
}

// 再戦時に飛行状態と耐久値を開始状態へ戻す
void Fighter::Reset(const Vector3 &position, float yaw, float speed) {
	position_ = position;
	yaw_ = yaw;
	pitch_ = roll_ = 0.0f;
	speed_ = speed;
	health_ = 3;
}

//===============
// 表示
//===============

// 飛行状態をモデルのTransformへ変換して更新する
void Fighter::UpdateVisual(const Camera &camera) {
	auto &transform = object_.GetTransform();
	transform.translate = position_;
	transform.rotate = {-pitch_, yaw_ + std::numbers::pi_v<float>, roll_};
	transform.scale = {1.8f, 1.8f, 1.8f};
	object_.Update(camera);
}

// 撃墜済みの機体は描画しない
void Fighter::Draw() const {
	if (IsAlive()) {
		object_.Draw();
	}
}

//===============
// 戦闘・飛行状態
//===============

// 耐久値が0を下回らないようにダメージを適用する
void Fighter::ApplyDamage(const Damage &damage) {
	health_ = std::max(0, health_ - damage.amount);
}

// ヨー角とピッチ角から、機体が向いている単位ベクトルを求める
Vector3 Fighter::GetForward() const {
	return DirectionFromYawPitch(yaw_, pitch_);
}
