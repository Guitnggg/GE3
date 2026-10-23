#include "application/effect/CombatEffectSystem.h"

#include "application/effect/EffectPreset.h"
#include "engine/3D/camera/Camera.h"
#include "engine/assets/AssetManager.h"
#include "engine/math/SpatialMath.h"
#include "engine/scene/SceneContext.h"

//===============
// 初期化・リセット
//===============

// 戦闘演出で共有するテクスチャとパーティクル設定を準備する
void CombatEffectSystem::Initialize(const SceneContext &context) {
	const auto texture = context.assets->LoadTexture("textures/particleSoft.png");
	particles_.Initialize(context.directXCommon, context.gpuParticlePipeline,
	                      context.textureManager, texture, 2048);
	particles_.SetPreset(EffectPreset::MissileTrail());
}

// 再戦時に残っているパーティクルとシードを破棄する
void CombatEffectSystem::Reset() {
	particles_.Reset();
	seed_ = 1;
}

//===============
// パーティクル放出
//===============

// ミサイル後方へ少量の航跡パーティクルを連続放出する
void CombatEffectSystem::EmitMissileTrail(const Vector3 &position, const Vector3 &velocity) {
	GPUParticleEmitData emit{};
	emit.position = SpatialMath::Add(position, SpatialMath::Scale(SpatialMath::NormalizeOr(velocity), -1.5f));
	emit.count = 2;
	emit.minVelocity = {-1.2f, -1.2f, -1.2f};
	emit.maxVelocity = {1.2f, 1.2f, 1.2f};
	emit.positionSpread = {0.3f, 0.3f, 0.3f};
	emit.seed = seed_++;
	particles_.Emit(emit);
}

// 命中位置を中心に全方向へ爆発パーティクルを放出する
void CombatEffectSystem::EmitExplosion(const Vector3 &position) {
	GPUParticleEmitData emit{};
	emit.position = position;
	emit.count = 72;
	emit.minVelocity = {-18.0f, -18.0f, -18.0f};
	emit.maxVelocity = {18.0f, 18.0f, 18.0f};
	emit.positionSpread = {1.0f, 1.0f, 1.0f};
	emit.seed = seed_++;
	particles_.Emit(emit);
}

//===============
// 更新・描画
//===============

// GPUパーティクルのシミュレーションを進めて描画する
void CombatEffectSystem::UpdateAndDraw(const Camera &camera, float deltaTime) {
	particles_.Update(deltaTime);
	particles_.Draw(camera);
}
