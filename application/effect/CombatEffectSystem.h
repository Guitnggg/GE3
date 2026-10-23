#pragma once

#include "engine/effects/particle/GPUParticleSystem.h"

#include <cstdint>

class Camera;
struct SceneContext;

/// <summary>
/// ミサイル航跡と命中時の爆発パーティクルを管理するクラス
/// </summary>
class CombatEffectSystem final {
  public:
	/// <summary>
	/// 戦闘用パーティクルシステムを初期化する
	/// </summary>
	void Initialize(const SceneContext &context);

	/// <summary>
	/// 表示中のパーティクルと乱数シードを初期状態へ戻す
	/// </summary>
	void Reset();

	/// <summary>
	/// ミサイル後方へ航跡パーティクルを発生させる
	/// </summary>
	void EmitMissileTrail(const Vector3 &position, const Vector3 &velocity);

	/// <summary>
	/// 指定位置へ爆発パーティクルを発生させる
	/// </summary>
	void EmitExplosion(const Vector3 &position);

	/// <summary>
	/// パーティクルを更新して描画する
	/// </summary>
	void UpdateAndDraw(const Camera &camera, float deltaTime);

  private:
	GPUParticleSystem particles_; // GPUで更新・描画するパーティクル群
	uint32_t seed_ = 1;           // 放出ごとに変化させる乱数シード
};
