#include "application/controller/PlayerController.h"

#include "application/actor/Fighter.h"
#include "engine/input/Input.h"
#include "engine/math/SpatialMath.h"

#include <algorithm>
#include <cmath>

// 入力を機体の姿勢、速度、位置へ順番に反映する
void PlayerController::Update(Fighter& fighter, Input& input, float deltaTime) const {
    constexpr float turnSpeed = 1.15f;

    // 機首方向を操作する
    if (input.PushKey(DIK_A)) {
        fighter.yaw_ -= turnSpeed * deltaTime;
    }
    if (input.PushKey(DIK_D)) {
        fighter.yaw_ += turnSpeed * deltaTime;
    }
    if (input.PushKey(DIK_W)) {
        fighter.pitch_ += turnSpeed * 0.75f * deltaTime;
    }
    if (input.PushKey(DIK_S)) {
        fighter.pitch_ -= turnSpeed * 0.75f * deltaTime;
    }

    // ロール操作する
    if (input.PushKey(DIK_Q)) {
        fighter.roll_ -= 1.7f * deltaTime;
    }
    if (input.PushKey(DIK_E)) {
        fighter.roll_ += 1.7f * deltaTime;
    }

    // 入力を離したロールは徐々に水平へ戻し、ピッチは反転しない範囲へ制限する。
    fighter.roll_ *= std::pow(0.18f, deltaTime);
    fighter.pitch_ = std::clamp(fighter.pitch_, -1.15f, 1.15f);

    // スロットル操作を許容速度内へ収める。
    if (input.PushKey(DIK_LSHIFT) || input.PushKey(DIK_RSHIFT)) {
        fighter.speed_ += 30.0f * deltaTime;
    }
    if (input.PushKey(DIK_LCONTROL) || input.PushKey(DIK_RCONTROL)) {
        fighter.speed_ -= 30.0f * deltaTime;
    }
    fighter.speed_ = std::clamp(fighter.speed_, 25.0f, 95.0f);

    // 現在の機首方向へ前進し、試作空域の地面より下へ落ちないようにする。
    fighter.position_ = SpatialMath::Add(
        fighter.position_, SpatialMath::Scale(fighter.GetForward(), fighter.speed_ * deltaTime));
    fighter.position_.y = std::max(fighter.position_.y, 3.0f);
}
