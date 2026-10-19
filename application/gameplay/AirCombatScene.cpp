#include "application/gameplay/AirCombatScene.h"

#include "application/gameplay/AirCombatMath.h"
#include "engine/3D/object/Object3dCommon.h"
#include "engine/assets/AssetManager.h"
#include "engine/core/timing/Time.h"
#include "engine/debug/Profiler.h"
#include "engine/input/Input.h"

#include <algorithm>

void AirCombatScene::OnInitialize() {
	const SceneContext &context = GetContext();
	fighterModel_ = context.assets->LoadModel("models/player/player.obj");
	player_.Initialize(context.object3dCommon, context.textureManager, fighterModel_, false);
	enemy_.Initialize(context.object3dCommon, context.textureManager, fighterModel_, true);
	missiles_.Initialize(context);
	camera_.Initialize();
	hud_.Initialize(*context.assets, context.spriteCommon, context.textureManager);
	ResetBattle();
}

void AirCombatScene::OnUpdate() {
	PROFILE_SCOPE("Air Combat Update");
	const SceneContext &context = GetContext();
	const float deltaTime = std::min(context.time->GetDeltaTime(), 1.0f / 20.0f);
	if (context.input->TriggerKey(DIK_R) && battleEnded_) ResetBattle();
	if (!battleEnded_) {
		player_.UpdatePlayer(*context.input, deltaTime);
		enemy_.UpdateEnemy(player_, deltaTime);
		lockedOn_ = hud_.UpdateLock(player_, enemy_, camera_.GetCamera(), context.time->GetElapsedTime());
		battleEnded_ = missiles_.Update(*context.input, player_, enemy_, lockedOn_, deltaTime);
	}
	camera_.Update(player_, deltaTime);
	UpdateVisuals();
}

void AirCombatScene::OnDraw() {
	PROFILE_SCOPE("Air Combat Draw");
	GetContext().object3dCommon->CommonDrawSetting();
	player_.Draw();
	enemy_.Draw();
	missiles_.Draw(camera_.GetCamera(), GetContext().time->GetDeltaTime());
	hud_.Draw(player_, enemy_, lockedOn_, battleEnded_);
}

void AirCombatScene::ResetBattle() {
	player_.Reset({0.0f, 30.0f, -85.0f}, 0.0f, 55.0f);
	enemy_.Reset({0.0f, 42.0f, 170.0f}, AirCombatMath::kPi, 38.0f);
	missiles_.Reset();
	lockedOn_ = battleEnded_ = false;
	camera_.Reset(player_);
	UpdateVisuals();
}

void AirCombatScene::UpdateVisuals() {
	player_.UpdateVisual(camera_.GetCamera());
	if (enemy_.IsAlive()) enemy_.UpdateVisual(camera_.GetCamera());
	missiles_.UpdateVisuals(camera_.GetCamera());
}
