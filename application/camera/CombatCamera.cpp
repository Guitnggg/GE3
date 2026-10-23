#include "application/camera/CombatCamera.h"

#include "application/actor/Fighter.h"
#include "engine/core/WinApp.h"
#include "engine/math/SpatialMath.h"

#include <algorithm>
#include <cmath>
#include <numbers>

using namespace SpatialMath;

//===============
// 初期化・リセット
//===============

// ゲーム画面に合わせて透視投影を設定する
void CombatCamera::Initialize() {
	camera_.SetAspectRatio(static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight));
	camera_.SetNearClip(0.1f);
	camera_.SetFarClip(4000.0f);
}

// 補間を行わず、プレイヤー後方の初期位置へ配置する
void CombatCamera::Reset(const Fighter &player) {
	position_ = Add(Add(player.GetPosition(), Scale(player.GetForward(), -18.0f)), {0.0f, 6.5f, 0.0f});
	Update(player, 1.0f);
}

//===============
// 追従更新
//===============

// プレイヤー後方を追従し、速度とロールを画角と傾きへ反映する
void CombatCamera::Update(const Fighter &player, float deltaTime) {
	const Vector3 forward = player.GetForward();

	// 指数補間により、フレームレートに依存しない滑らかな追従を行う。
	const Vector3 desired = Add(Add(player.GetPosition(), Scale(forward, -18.0f)), {0.0f, 6.5f, 0.0f});
	position_ = Add(position_, Scale(Subtract(desired, position_), 1.0f - std::exp(-7.0f * deltaTime)));

	// 機体より少し前を注視して進行方向を見やすくする。
	const Vector3 look = NormalizeOr(Subtract(Add(player.GetPosition(), Scale(forward, 18.0f)), position_));
	camera_.SetTranslate(position_);
	camera_.SetRotate({-std::asin(std::clamp(look.y, -1.0f, 1.0f)), std::atan2(look.x, look.z), player.GetRoll() * 0.12f});

	// 高速時は視野角を広げ、速度感を強調する。
	camera_.SetFovY((60.0f + (player.GetSpeed() - 25.0f) * 0.16f) * std::numbers::pi_v<float> / 180.0f);
	camera_.Update();
}
