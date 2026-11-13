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
	trailParticles_.Initialize(context.directXCommon, context.gpuParticlePipeline,
	                           context.textureManager, texture, 2048);
	fireParticles_.Initialize(context.directXCommon, context.gpuParticlePipeline,
	                          context.textureManager, texture, 1536);
	sparkParticles_.Initialize(context.directXCommon, context.gpuParticlePipeline,
	                           context.textureManager, texture, 1536);
	smokeParticles_.Initialize(context.directXCommon, context.gpuParticlePipeline,
	                           context.textureManager, texture, 768);
	trailParticles_.SetPreset(EffectPreset::MissileTrail());
	fireParticles_.SetPreset(EffectPreset::ExplosionFire());
	sparkParticles_.SetPreset(EffectPreset::ExplosionSparks());
	smokeParticles_.SetPreset(EffectPreset::ExplosionSmoke());
}

// 再戦時に残っているパーティクルとシードを破棄する
void CombatEffectSystem::Reset() {
	trailParticles_.Reset();
	fireParticles_.Reset();
	sparkParticles_.Reset();
	smokeParticles_.Reset();
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
	trailParticles_.Emit(emit);
}

// 命中位置を中心に全方向へ爆発パーティクルを放出する
void CombatEffectSystem::EmitExplosion(const Vector3 &position) {
	GPUParticleEmitData emit{};
	emit.position = position;
	emit.count = 96;
	emit.minVelocity = {-15.0f, -15.0f, -15.0f};
	emit.maxVelocity = {15.0f, 15.0f, 15.0f};
	emit.positionSpread = {1.5f, 1.5f, 1.5f};
	emit.seed = seed_++;
	fireParticles_.Emit(emit);
}

// 撃墜位置へ火球、閃光、火花、煙を重ねて大規模爆発を作る
void CombatEffectSystem::EmitDestruction(const Vector3 &position) {
	GPUParticleEmitData fire{};
	fire.position = position;
	fire.count = 360;
	fire.minVelocity = {-28.0f, -24.0f, -28.0f};
	fire.maxVelocity = {28.0f, 30.0f, 28.0f};
	fire.positionSpread = {4.0f, 4.0f, 4.0f};
	fire.seed = seed_++;
	fireParticles_.Emit(fire);

	// 低速かつ大型の粒子を中心に残し、衝撃波のような一瞬の閃光を作る。
	GPUParticleEmitData flash{};
	flash.position = position;
	flash.count = 72;
	flash.minVelocity = {-4.0f, -4.0f, -4.0f};
	flash.maxVelocity = {4.0f, 4.0f, 4.0f};
	flash.positionSpread = {1.0f, 1.0f, 1.0f};
	flash.seed = seed_++;
	fireParticles_.Emit(flash);

	GPUParticleEmitData sparks{};
	sparks.position = position;
	sparks.count = 420;
	sparks.minVelocity = {-65.0f, -48.0f, -65.0f};
	sparks.maxVelocity = {65.0f, 70.0f, 65.0f};
	sparks.positionSpread = {2.5f, 2.5f, 2.5f};
	sparks.seed = seed_++;
	sparkParticles_.Emit(sparks);

	GPUParticleEmitData smoke{};
	smoke.position = position;
	smoke.count = 210;
	smoke.minVelocity = {-12.0f, -5.0f, -12.0f};
	smoke.maxVelocity = {12.0f, 17.0f, 12.0f};
	smoke.positionSpread = {5.0f, 4.0f, 5.0f};
	smoke.seed = seed_++;
	smokeParticles_.Emit(smoke);
}

//===============
// 更新・描画
//===============

// GPUパーティクルのシミュレーションを進めて描画する
void CombatEffectSystem::UpdateAndDraw(const Camera &camera, float deltaTime) {
	trailParticles_.Update(deltaTime);
	fireParticles_.Update(deltaTime);
	sparkParticles_.Update(deltaTime);
	smokeParticles_.Update(deltaTime);
	trailParticles_.Draw(camera);
	smokeParticles_.Draw(camera);
	fireParticles_.Draw(camera);
	sparkParticles_.Draw(camera);
}
