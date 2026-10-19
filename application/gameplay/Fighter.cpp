#include "application/gameplay/Fighter.h"

#include "application/gameplay/AirCombatMath.h"
#include "engine/3D/camera/Camera.h"
#include "engine/3D/model/Model.h"
#include "engine/input/Input.h"

#include <algorithm>
#include <cmath>

using namespace AirCombatMath;

void Fighter::Initialize(Object3dCommon *object3dCommon, TextureManager *textureManager,
                         const std::shared_ptr<Model> &model, bool enemy) {
	object_.Initialize(object3dCommon, textureManager, model);
	if (enemy) object_.GetMaterialData()->color = {1.0f, 0.28f, 0.2f, 1.0f};
}

void Fighter::Reset(const Vector3 &position, float yaw, float speed) {
	position_ = position;
	yaw_ = yaw;
	pitch_ = roll_ = 0.0f;
	speed_ = speed;
	health_ = 3;
}

void Fighter::UpdatePlayer(Input &input, float deltaTime) {
	constexpr float turnSpeed = 1.15f;
	if (input.PushKey(DIK_A)) yaw_ -= turnSpeed * deltaTime;
	if (input.PushKey(DIK_D)) yaw_ += turnSpeed * deltaTime;
	if (input.PushKey(DIK_W)) pitch_ += turnSpeed * 0.75f * deltaTime;
	if (input.PushKey(DIK_S)) pitch_ -= turnSpeed * 0.75f * deltaTime;
	if (input.PushKey(DIK_Q)) roll_ -= 1.7f * deltaTime;
	if (input.PushKey(DIK_E)) roll_ += 1.7f * deltaTime;
	roll_ *= std::pow(0.18f, deltaTime);
	pitch_ = std::clamp(pitch_, -1.15f, 1.15f);
	if (input.PushKey(DIK_LSHIFT) || input.PushKey(DIK_RSHIFT)) speed_ += 30.0f * deltaTime;
	if (input.PushKey(DIK_LCONTROL) || input.PushKey(DIK_RCONTROL)) speed_ -= 30.0f * deltaTime;
	speed_ = std::clamp(speed_, 25.0f, 95.0f);
	position_ = Add(position_, Scale(GetForward(), speed_ * deltaTime));
	position_.y = std::max(position_.y, 3.0f);
}

void Fighter::UpdateEnemy(const Fighter &target, float deltaTime) {
	if (!IsAlive()) return;
	const Vector3 direction = Normalize(Subtract(target.position_, position_));
	const float desiredYaw = std::atan2(direction.x, direction.z);
	const float desiredPitch = std::asin(std::clamp(direction.y, -1.0f, 1.0f));
	yaw_ = MoveAngleTowards(yaw_, desiredYaw, 0.72f * deltaTime);
	pitch_ = MoveAngleTowards(pitch_, desiredPitch, 0.48f * deltaTime);
	roll_ = std::clamp(-std::remainder(desiredYaw - yaw_, kPi * 2.0f) * 1.4f, -0.8f, 0.8f);
	position_ = Add(position_, Scale(GetForward(), speed_ * deltaTime));
	position_.y = std::max(position_.y, 5.0f);
}

void Fighter::UpdateVisual(const Camera &camera) {
	auto &transform = object_.GetTransform();
	transform.translate = position_;
	transform.rotate = {-pitch_, yaw_ + kPi, roll_};
	transform.scale = {1.8f, 1.8f, 1.8f};
	object_.Update(camera);
}

void Fighter::Draw() const { if (IsAlive()) object_.Draw(); }
void Fighter::Damage() { if (health_ > 0) --health_; }
Vector3 Fighter::GetForward() const { return Forward(yaw_, pitch_); }
