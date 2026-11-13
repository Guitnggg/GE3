#include "application/controller/PlayerController.h"

#include "application/actor/Fighter.h"
#include "engine/input/Input.h"
#include "engine/math/SpatialMath.h"

#include <algorithm>
#include <cmath>

// 入力を機体の姿勢、速度、位置へ順番に反映する
void PlayerController::Update(Fighter& fighter, Input& input, float deltaTime) const {
    constexpr float turnSpeed = 1.15f;
    constexpr float mouseRollSensitivity = 0.012f;
    constexpr LONG maxMouseDelta = 50;
    constexpr uint32_t leftMouseButton = 0;
    constexpr uint32_t rightMouseButton = 1;

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

    // マウスの横移動量を機体のロールへ反映する。
    // 極端な入力値は制限し、ウィンドウ切り替え直後などの急回転を防ぐ。
    const LONG mouseDeltaX = std::clamp(input.GetMouseDeltaX(), -maxMouseDelta, maxMouseDelta);
    fighter.roll_ += static_cast<float>(mouseDeltaX) * mouseRollSensitivity;

    // マウスを止めたロールは徐々に水平へ戻し、姿勢は操作可能な範囲へ制限する。
    fighter.roll_ *= std::pow(0.18f, deltaTime);
    fighter.roll_ = std::clamp(fighter.roll_, -1.15f, 1.15f);
    fighter.pitch_ = std::clamp(fighter.pitch_, -1.15f, 1.15f);

    // 左クリックで加速、右クリックで減速する。
    if (input.PushMouseButton(leftMouseButton)) {
        fighter.speed_ += 30.0f * deltaTime;
    }
    if (input.PushMouseButton(rightMouseButton)) {
        fighter.speed_ -= 30.0f * deltaTime;
    }
    fighter.speed_ = std::clamp(fighter.speed_, 25.0f, 95.0f);

    // 現在の機首方向へ前進し、試作空域の地面より下へ落ちないようにする。
    fighter.position_ = SpatialMath::Add(
        fighter.position_, SpatialMath::Scale(fighter.GetForward(), fighter.speed_ * deltaTime));
    fighter.position_.y = std::max(fighter.position_.y, 3.0f);
}
