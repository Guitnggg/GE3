#include "application/combat/LockOnSystem.h"

#include "application/actor/Fighter.h"
#include "engine/3D/camera/Camera.h"
#include "engine/core/WinApp.h"
#include "engine/math/SpatialMath.h"

// ターゲットが射程、機首前方、画面内の全条件を満たすか判定する
bool LockOnSystem::Update(const Fighter &source, const Fighter &target, const Camera &camera) {
	if (!target.IsAlive()) {
		lockedOn_ = false;
		return false;
	}

	// 機首との内積を使い、正面のロックオン可能角度内にいるかを調べる。
	const Vector3 toTarget = SpatialMath::Subtract(target.GetPosition(), source.GetPosition());
	lockedOn_ = SpatialMath::Length(toTarget) < 650.0f &&
	            SpatialMath::Dot(source.GetForward(), SpatialMath::NormalizeOr(toTarget)) > 0.55f &&
	            SpatialMath::WorldToScreen(target.GetPosition(), camera.GetViewProjectionMatrix(),
	                                       static_cast<float>(WinApp::kClientWidth),
	                                       static_cast<float>(WinApp::kClientHeight), screenPosition_, 0.15f);
	return lockedOn_;
}

// 戦闘リセット時に前回のターゲット情報を破棄する
void LockOnSystem::Reset() {
	lockedOn_ = false;
	screenPosition_ = {};
}
