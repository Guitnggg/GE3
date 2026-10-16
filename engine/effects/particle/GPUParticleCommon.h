#pragma once

#include "engine/math/MathTypes.h"

#include <cstdint>

enum class GPUParticleBlendMode {
	Alpha,    // アルファ値で背景色と合成する
	Additive, // 背景色へ加算して発光表現を行う
};

/// <summary>
/// GPUパーティクルの運動と見た目を演出単位で定義する設定値。
/// </summary>
struct GPUParticlePreset {
	Vector3 acceleration{};                                       // 全粒子へ1秒ごとに加える加速度
	float drag = 0.0f;                                            // 粒子速度を減衰させる抵抗係数
	Vector4 startColor{1.0f, 1.0f, 1.0f, 1.0f};                   // 生成時に使用する色と不透明度
	Vector4 endColor{1.0f, 1.0f, 1.0f, 0.0f};                     // 消滅時に近づける色と不透明度
	Vector2 startSize{0.25f, 0.25f};                              // 生成時の幅と高さ
	Vector2 endSize{};                                            // 消滅時に近づける幅と高さ
	float minLifetime = 0.5f;                                     // ランダムな生存時間の下限秒数
	float maxLifetime = 1.0f;                                     // ランダムな生存時間の上限秒数
	GPUParticleBlendMode blendMode = GPUParticleBlendMode::Alpha; // 描画時に使用する色合成方式
};

/// <summary>
/// 1回分の生成数と、GPUでランダム化する各パラメーター範囲。
/// </summary>
struct GPUParticleEmitData {
	Vector3 position{};                       // 粒子生成範囲の中心位置
	uint32_t count = 1;                       // この命令で生成する粒子数
	Vector3 minVelocity{-1.0f, -1.0f, -1.0f}; // ランダムな初速度の下限
	Vector3 maxVelocity{1.0f, 1.0f, 1.0f};    // ランダムな初速度の上限
	Vector3 positionSpread{};                 // 中心位置から各軸へ広げる生成範囲
	uint32_t seed = 0;                        // GPU上のランダム値生成に使用する種
};
