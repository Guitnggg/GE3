#include "application/gameplay/CombatCamera.h"

#include "application/gameplay/AirCombatMath.h"
#include "application/gameplay/Fighter.h"

#include <algorithm>
#include <cmath>

using namespace AirCombatMath;

void CombatCamera::Initialize() {
	camera_.SetAspectRatio(kScreenWidth / kScreenHeight);
	camera_.SetNearClip(0.1f);
	camera_.SetFarClip(4000.0f);
}

void CombatCamera::Reset(const Fighter &player) {
	position_ = Add(Add(player.GetPosition(), Scale(player.GetForward(), -18.0f)), {0.0f, 6.5f, 0.0f});
	Update(player, 1.0f);
}

void CombatCamera::Update(const Fighter &player, float deltaTime) {
	const Vector3 forward = player.GetForward();
	const Vector3 desired = Add(Add(player.GetPosition(), Scale(forward, -18.0f)), {0.0f, 6.5f, 0.0f});
	position_ = Add(position_, Scale(Subtract(desired, position_), 1.0f - std::exp(-7.0f * deltaTime)));
	const Vector3 look = Normalize(Subtract(Add(player.GetPosition(), Scale(forward, 18.0f)), position_));
	camera_.SetTranslate(position_);
	camera_.SetRotate({-std::asin(std::clamp(look.y, -1.0f, 1.0f)), std::atan2(look.x, look.z), player.GetRoll() * 0.12f});
	camera_.SetFovY((60.0f + (player.GetSpeed() - 25.0f) * 0.16f) * kPi / 180.0f);
	camera_.Update();
}
