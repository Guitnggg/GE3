#include "application/controller/EnemyController.h"

#include "application/actor/Fighter.h"
#include "engine/math/MathUtility.h"
#include "engine/math/SpatialMath.h"

#include <algorithm>
#include <cmath>

// ターゲットへの方向を求め、旋回速度の範囲内で敵機を追尾させる
void EnemyController::Update(Fighter &fighter, const Fighter &target, float deltaTime) const {
	if (!fighter.IsAlive()) {
		return;
	}

	// 敵機からターゲットへ向かう理想のヨー角とピッチ角を求める。
	const Vector3 direction = SpatialMath::NormalizeOr(
	    SpatialMath::Subtract(target.GetPosition(), fighter.position_));
	const float desiredYaw = std::atan2(direction.x, direction.z);
	const float desiredPitch = std::asin(std::clamp(direction.y, -1.0f, 1.0f));

	// 瞬時に機首を向けず、機体ごとの旋回性能を表現する。
	fighter.yaw_ = MoveTowardsAngle(fighter.yaw_, desiredYaw, 0.72f * deltaTime);
	fighter.pitch_ = MoveTowardsAngle(fighter.pitch_, desiredPitch, 0.48f * deltaTime);
	fighter.roll_ = std::clamp(-DeltaAngle(fighter.yaw_, desiredYaw) * 1.4f, -0.8f, 0.8f);

	// 更新後の機首方向へ前進する。
	fighter.position_ = SpatialMath::Add(
	    fighter.position_, SpatialMath::Scale(fighter.GetForward(), fighter.speed_ * deltaTime));
	fighter.position_.y = std::max(fighter.position_.y, 5.0f);
}
