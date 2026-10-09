#pragma once

#include "engine/effects/particle/GPUParticleCommon.h"

#include <cstdint>

class GPUParticleSystem;

/// <summary>
/// GPUパーティクルの生成内容と発生間隔をCPU側で制御するエミッター。
/// </summary>
class GPUParticleEmitter final {
  public:
	/// <summary>
	/// 発生先、発生内容、発生間隔を設定する。
	/// </summary>
	void Initialize(GPUParticleSystem *system, const GPUParticleEmitData &emitTemplate, float intervalSeconds);
	/// <summary>
	/// 有効な間、経過時間に応じて指定位置へパーティクルを発生させる。
	/// </summary>
	void Update(float deltaTime, const Vector3 &position);
	/// <summary>
	/// 時間間隔に関係なく指定位置へ1回発生させる。
	/// </summary>
	void EmitOnce(const Vector3 &position);
	/// <summary>
	/// 発生タイマーと通し番号を初期状態へ戻す。
	/// </summary>
	void Reset();
	/// <summary>
	/// 以降の発生に使用する設定テンプレートを変更する。
	/// </summary>
	void SetEmitTemplate(const GPUParticleEmitData &emitTemplate) {
		emitTemplate_ = emitTemplate;
	}
	/// <summary>
	/// 継続発生の時間間隔を秒単位で変更する。
	/// </summary>
	void SetInterval(float intervalSeconds);
	/// <summary>
	/// 時間経過による継続発生の有効・無効を切り替える。
	/// </summary>
	void SetActive(bool active) {
		isActive_ = active;
	}
	/// <summary>
	/// 継続発生が有効か取得する。
	/// </summary>
	bool IsActive() const {
		return isActive_;
	}

  private:
	GPUParticleSystem *system_ = nullptr; // 発生要求の送信先。所有しない
	GPUParticleEmitData emitTemplate_{};
	float interval_ = 0.0f;
	float elapsed_ = 0.0f;          // 前回発生後に蓄積した秒数
	uint32_t emissionSequence_ = 0; // 発生ごとに乱数系列を変える通し番号
	bool isActive_ = false;
};
