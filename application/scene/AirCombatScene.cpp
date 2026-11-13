#include "application/scene/AirCombatScene.h"

#include "engine/3D/object/Object3dCommon.h"
#include "engine/assets/AssetManager.h"
#include "engine/core/timing/Time.h"
#include "engine/debug/Profiler.h"
#include "engine/input/Input.h"

#include <algorithm>
#include <numbers>

//===============
// シーン初期化
//===============

// 空戦を構成するアセットと各サブシステムを準備する
void AirCombatScene::OnInitialize() {
	const SceneContext &context = GetContext();

	// 現在は同じモデルを色分けしてプレイヤー機と敵機に使用する。
	fighterModel_ = context.assets->LoadModel("models/player/player.obj");
	player_.Initialize(context.object3dCommon, context.textureManager, fighterModel_, Team::Player);
	enemy_.Initialize(context.object3dCommon, context.textureManager, fighterModel_,
	                  Team::Enemy, {1.0f, 0.28f, 0.2f, 1.0f});

	missiles_.Initialize(context);
	camera_.Initialize();
	hud_.Initialize(*context.assets, context.spriteCommon, context.textureManager);

	ResetBattle();
}

//===============
// フレーム更新
//===============

// 入力、敵AI、ロックオン、ミサイル、カメラの順に更新する
void AirCombatScene::OnUpdate() {
	PROFILE_SCOPE("Air Combat Update");

	const SceneContext &context = GetContext();
	// フレーム落ちで機体やミサイルが大きく飛び越えないよう、更新幅に上限を設ける。
	const float deltaTime = std::min(context.time->GetDeltaTime(), 1.0f / 20.0f);

	if (context.input->TriggerKey(DIK_R) && battleEnded_) {
		ResetBattle();
	}

	if (!battleEnded_) {
		// 機体の種類ではなく、割り当てたコントローラーによって操縦方法を切り替える。
		playerController_.Update(player_, *context.input, deltaTime);
		enemyController_.Update(enemy_, player_, deltaTime);

		// ロックオン判定とHUD表示を同じフレームの結果で同期させる。
		lockedOn_ = lockOnSystem_.Update(player_, enemy_, camera_.GetCamera());
		hud_.UpdateLockMarker(lockedOn_, lockOnSystem_.GetScreenPosition(), context.time->GetElapsedTime());

		battleEnded_ = missiles_.Update(*context.input, player_, enemy_, lockedOn_, deltaTime);
	}

	camera_.Update(player_, deltaTime);
	UpdateVisuals();
}

//===============
// 描画
//===============

// 3Dオブジェクトを描画した後、戦闘情報をHUDへ重ねる
void AirCombatScene::OnDraw() {
	PROFILE_SCOPE("Air Combat Draw");
	GetContext().object3dCommon->CommonDrawSetting();
	player_.Draw();
	enemy_.Draw();
	missiles_.Draw(camera_.GetCamera(), GetContext().time->GetDeltaTime());
	hud_.Draw(player_, enemy_, lockedOn_, battleEnded_);
	if (editor_.Draw(player_, enemy_, missiles_, *GetContext().time, lockedOn_, battleEnded_)) {
		ResetBattle();
	}
	battleEnded_ = battleEnded_ || !player_.IsAlive() || !enemy_.IsAlive();
}

//===============
// 戦闘状態の管理
//===============

// 新しい戦闘を開始できるよう、動的な状態をすべて初期化する
void AirCombatScene::ResetBattle() {
	player_.Reset({0.0f, 30.0f, -85.0f}, 0.0f, 55.0f);
	enemy_.Reset({0.0f, 42.0f, 170.0f}, std::numbers::pi_v<float>, 38.0f);
	missiles_.Reset();
	lockOnSystem_.Reset();
	lockedOn_ = battleEnded_ = false;
	camera_.Reset(player_);
	UpdateVisuals();
}

// 機体とミサイルの論理状態を描画用オブジェクトへ反映する
void AirCombatScene::UpdateVisuals() {
	player_.UpdateVisual(camera_.GetCamera());
	if (enemy_.IsAlive()) {
		enemy_.UpdateVisual(camera_.GetCamera());
	}

	missiles_.UpdateVisuals(camera_.GetCamera());
}
