#include "application/weapon/MissileSystem.h"

#include "application/actor/Fighter.h"
#include "application/combat/Damage.h"
#include "engine/3D/camera/Camera.h"
#include "engine/assets/AssetManager.h"
#include "engine/input/Input.h"
#include "engine/math/SpatialMath.h"
#include "engine/scene/SceneContext.h"

#include <algorithm>

//===============
// 初期化・リセット
//===============

// 全ミサイルで共有するモデルと戦闘演出を準備する
void MissileSystem::Initialize(const SceneContext &context) {
	context_ = &context;
	model_ = context.assets->LoadModel("models/missile/missile.obj");
	effects_.Initialize(context);
}

// 飛行中のミサイルを破棄し、発射タイマーを開始状態へ戻す
void MissileSystem::Reset() {
	missiles_.clear();
	effects_.Reset();
	playerCooldown_ = 0.0f;
	enemyCooldown_ = 1.8f;
}

//===============
// 戦闘更新
//===============

// 発射、誘導、移動、命中、寿命切れを1フレーム分処理する
bool MissileSystem::Update(Input &input, Fighter &player, Fighter &enemy, bool lockedOn, float deltaTime) {
	// 機体ごとの再発射待ち時間を減らす。
	playerCooldown_ = std::max(0.0f, playerCooldown_ - deltaTime);
	enemyCooldown_ = std::max(0.0f, enemyCooldown_ - deltaTime);
	if (input.TriggerKey(DIK_SPACE) && playerCooldown_ <= 0.0f) {
		Spawn(player);
		playerCooldown_ = 0.45f;
	}

	// 敵機はプレイヤーが射程内かつ前方にいる場合だけ発射する。
	const Vector3 enemyToPlayer = SpatialMath::NormalizeOr(
	    SpatialMath::Subtract(player.GetPosition(), enemy.GetPosition()));
	if (enemy.IsAlive() && enemyCooldown_ <= 0.0f &&
	    SpatialMath::Dot(enemy.GetForward(), enemyToPlayer) > 0.72f &&
	    SpatialMath::Length(SpatialMath::Subtract(player.GetPosition(), enemy.GetPosition())) < 500.0f) {
		Spawn(enemy);
		enemyCooldown_ = 2.3f;
	}

	// 飛行中の各ミサイルを対象へ誘導して前進させる。
	for (auto &missile : missiles_) {
		const bool firedByPlayer = missile->GetOwnerTeam() == Team::Player;
		const Fighter &target = firedByPlayer ? enemy : player;
		if (target.IsAlive()) {
			const Vector3 desired = SpatialMath::NormalizeOr(
			    SpatialMath::Subtract(target.GetPosition(), missile->GetPosition()));
			const Vector3 current = SpatialMath::NormalizeOr(missile->GetVelocity());
			const float homing = firedByPlayer && !lockedOn ? 0.65f : 2.4f;
			missile->SetVelocity(SpatialMath::Scale(
			    SpatialMath::NormalizeOr(SpatialMath::Add(
			        current, SpatialMath::Scale(SpatialMath::Subtract(desired, current), homing * deltaTime))),
			    115.0f));
		}
		missile->Advance(deltaTime);
		effects_.EmitMissileTrail(missile->GetPosition(), missile->GetVelocity());
	}

	// 対象との距離が命中半径を下回ったらダメージと爆発を発生させる。
	bool battleEnded = false;
	for (auto &missile : missiles_) {
		Fighter &target = missile->GetOwnerTeam() == Team::Player ? enemy : player;
		if (target.IsAlive() && SpatialMath::Length(
		        SpatialMath::Subtract(target.GetPosition(), missile->GetPosition())) < 5.5f) {
			target.ApplyDamage(Damage{1});
			missile->Expire();
			effects_.EmitExplosion(target.GetPosition());
			battleEnded |= !target.IsAlive();
		}
	}

	// 命中済み、または最大飛行時間を超えたミサイルを破棄する。
	std::erase_if(missiles_, [](const auto &missile) { return missile->GetLife() > 7.0f; });
	return battleEnded;
}

//===============
// 表示
//===============

// 各ミサイルの論理状態を3Dモデルへ反映する
void MissileSystem::UpdateVisuals(const Camera &camera) {
	for (auto &missile : missiles_) {
		missile->UpdateVisual(camera);
	}
}

// ミサイルモデルを描画した後、航跡と爆発を描画する
void MissileSystem::Draw(const Camera &camera, float deltaTime) {
	for (const auto &missile : missiles_) {
		missile->Draw();
	}

	effects_.UpdateAndDraw(camera, deltaTime);
}

// 発射元の機首前方へ、所属チームを引き継いだミサイルを生成する
void MissileSystem::Spawn(const Fighter &owner) {
	auto missile = std::make_unique<Missile>();
	missile->Initialize(
	    context_->object3dCommon, context_->textureManager, model_,
	    SpatialMath::Add(owner.GetPosition(), SpatialMath::Scale(owner.GetForward(), 5.0f)),
	    SpatialMath::Scale(owner.GetForward(), 115.0f), owner.GetTeam());
	missiles_.push_back(std::move(missile));
}
