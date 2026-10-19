#include "application/gameplay/MissileSystem.h"

#include "application/gameplay/AirCombatMath.h"
#include "application/gameplay/Fighter.h"
#include "engine/3D/camera/Camera.h"
#include "engine/assets/AssetManager.h"
#include "engine/input/Input.h"
#include "engine/scene/SceneContext.h"

#include <algorithm>
#include <cmath>

using namespace AirCombatMath;

void MissileSystem::Initialize(const SceneContext &context) {
	context_ = &context;
	model_ = context.assets->LoadModel("models/missile/missile.obj");
	const auto texture = context.assets->LoadTexture("textures/particleSoft.png");
	particles_.Initialize(context.directXCommon, context.gpuParticlePipeline, context.textureManager, texture, 2048);
	GPUParticlePreset preset{};
	preset.drag = 1.4f;
	preset.startColor = {1.0f, 0.65f, 0.18f, 0.85f};
	preset.endColor = {0.16f, 0.18f, 0.2f, 0.0f};
	preset.startSize = {1.3f, 1.3f};
	preset.endSize = {3.5f, 3.5f};
	preset.minLifetime = 0.35f;
	preset.maxLifetime = 0.8f;
	particles_.SetPreset(preset);
}

void MissileSystem::Reset() {
	missiles_.clear();
	particles_.Reset();
	playerCooldown_ = 0.0f;
	enemyCooldown_ = 1.8f;
}

bool MissileSystem::Update(Input &input, Fighter &player, Fighter &enemy, bool lockedOn, float deltaTime) {
	playerCooldown_ = std::max(0.0f, playerCooldown_ - deltaTime);
	enemyCooldown_ = std::max(0.0f, enemyCooldown_ - deltaTime);
	if (input.TriggerKey(DIK_SPACE) && playerCooldown_ <= 0.0f) {
		Spawn(player, true);
		playerCooldown_ = 0.45f;
	}
	const Vector3 enemyToPlayer = Normalize(Subtract(player.GetPosition(), enemy.GetPosition()));
	if (enemy.IsAlive() && enemyCooldown_ <= 0.0f && Dot(enemy.GetForward(), enemyToPlayer) > 0.72f &&
	    Length(Subtract(player.GetPosition(), enemy.GetPosition())) < 500.0f) {
		Spawn(enemy, false);
		enemyCooldown_ = 2.3f;
	}

	for (auto &missile : missiles_) {
		missile->life += deltaTime;
		const Fighter &target = missile->firedByPlayer ? enemy : player;
		if (target.IsAlive()) {
			const Vector3 desired = Normalize(Subtract(target.GetPosition(), missile->position));
			const Vector3 current = Normalize(missile->velocity);
			const float homing = missile->firedByPlayer && !lockedOn ? 0.65f : 2.4f;
			missile->velocity = Scale(Normalize(Add(current, Scale(Subtract(desired, current), homing * deltaTime))), 115.0f);
		}
		missile->position = Add(missile->position, Scale(missile->velocity, deltaTime));
		EmitTrail(missile->position, missile->velocity);
	}

	bool battleEnded = false;
	for (auto &missile : missiles_) {
		Fighter &target = missile->firedByPlayer ? enemy : player;
		if (target.IsAlive() && Length(Subtract(target.GetPosition(), missile->position)) < 5.5f) {
			target.Damage();
			missile->life = 99.0f;
			EmitExplosion(target.GetPosition());
			battleEnded |= !target.IsAlive();
		}
	}
	std::erase_if(missiles_, [](const auto &missile) { return missile->life > 7.0f; });
	return battleEnded;
}

void MissileSystem::UpdateVisuals(const Camera &camera) {
	for (auto &missile : missiles_) {
		const Vector3 direction = Normalize(missile->velocity);
		auto &transform = missile->object.GetTransform();
		transform.translate = missile->position;
		transform.rotate = {-std::asin(std::clamp(direction.y, -1.0f, 1.0f)), std::atan2(direction.x, direction.z), 0.0f};
		missile->object.Update(camera);
	}
}

void MissileSystem::Draw(const Camera &camera, float deltaTime) {
	for (const auto &missile : missiles_) missile->object.Draw();
	particles_.Update(deltaTime);
	particles_.Draw(camera);
}

void MissileSystem::Spawn(const Fighter &owner, bool firedByPlayer) {
	auto missile = std::make_unique<Missile>();
	missile->object.Initialize(context_->object3dCommon, context_->textureManager, model_);
	missile->object.GetTransform().scale = {0.24f, 0.24f, 0.24f};
	missile->firedByPlayer = firedByPlayer;
	missile->position = Add(owner.GetPosition(), Scale(owner.GetForward(), 5.0f));
	missile->velocity = Scale(owner.GetForward(), 115.0f);
	missiles_.push_back(std::move(missile));
}

void MissileSystem::EmitTrail(const Vector3 &position, const Vector3 &velocity) {
	GPUParticleEmitData emit{};
	emit.position = Add(position, Scale(Normalize(velocity), -1.5f));
	emit.count = 2;
	emit.minVelocity = {-1.2f, -1.2f, -1.2f};
	emit.maxVelocity = {1.2f, 1.2f, 1.2f};
	emit.positionSpread = {0.3f, 0.3f, 0.3f};
	emit.seed = particleSeed_++;
	particles_.Emit(emit);
}

void MissileSystem::EmitExplosion(const Vector3 &position) {
	GPUParticleEmitData emit{};
	emit.position = position;
	emit.count = 72;
	emit.minVelocity = {-18.0f, -18.0f, -18.0f};
	emit.maxVelocity = {18.0f, 18.0f, 18.0f};
	emit.positionSpread = {1.0f, 1.0f, 1.0f};
	emit.seed = particleSeed_++;
	particles_.Emit(emit);
}
