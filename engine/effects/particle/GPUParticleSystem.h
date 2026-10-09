#pragma once

#include "engine/effects/particle/GPUParticleCommon.h"

#include <cstdint>
#include <vector>

#include <d3d12.h>
#include <wrl.h>

class Camera;
class DirectXCommon;
class GPUParticlePipeline;
class TextureManager;

/// <summary>
/// GPU上で生成・更新・描画を完結する再利用可能なパーティクルプール。
/// 演出差分は継承ではなくPresetとEmitDataで与える。
/// </summary>
class GPUParticleSystem final {
  public:
	/// <summary>GPUパーティクルに必要なバッファと参照先を初期化する。</summary>
	void Initialize(DirectXCommon *dxCommon,
	                GPUParticlePipeline *pipeline,
	                TextureManager *textureManager,
	                uint32_t textureHandle,
	                uint32_t maxParticles = 4096);

	/// <summary>生成とシミュレーションに使用するパラメーター一式を設定する。</summary>
	void SetPreset(const GPUParticlePreset &preset);

	/// <summary>パーティクル生成要求を現在フレームの待機列へ追加する。</summary>
	void Emit(const GPUParticleEmitData &emitData);

	/// <summary>待機中の生成要求を反映し、GPU上の粒子状態を更新する。</summary>
	void Update(float deltaTime);

	/// <summary>生存中のパーティクルを指定カメラから描画する。</summary>
	void Draw(const Camera &camera);

	/// <summary>すべての粒子を未使用状態へ戻す。</summary>
	void Reset();

	/// <summary>同時に保持できるパーティクル数を返す。</summary>
	uint32_t GetMaxParticles() const {
		return maxParticles_;
	}

	/// <summary>現在使用しているパーティクル設定を返す。</summary>
	const GPUParticlePreset &GetPreset() const {
		return preset_;
	}

  private:
	static constexpr uint32_t kThreadGroupSize = 256;        // Compute Shaderの1グループ当たりのスレッド数
	static constexpr uint32_t kMaxEmitCommandsPerFrame = 64; // 1フレームで処理できる生成命令の上限

	struct alignas(16) Particle {
		Vector3 position{};    // ワールド空間上の現在位置
		float lifetime = 0.0f; // 消滅するまでの生存時間
		Vector3 velocity{};    // 1秒当たりの移動量
		float age = 0.0f;      // 生成後に経過した時間
		Vector4 startColor{};  // 生成時に使用する色
		Vector4 endColor{};    // 消滅時に近づける色
		Vector2 startSize{};   // 生成時の表示寸法
		Vector2 endSize{};     // 消滅時に近づける表示寸法
	};

	struct alignas(256) SimulationConstants {
		float deltaTime = 0.0f;    // 今回のシミュレーション時間
		uint32_t maxParticles = 0; // バッファに格納できる粒子数
		Vector2 padding{};         // GPU定数バッファ配置を揃える余白
		Vector3 acceleration{};    // 全粒子へ加える加速度
		float drag = 0.0f;         // 速度へ適用する抵抗係数
	};

	struct alignas(256) EmitConstants {
		Vector3 position{};       // 粒子生成の中心位置
		uint32_t count = 0;       // 今回生成する粒子数
		Vector3 minVelocity{};    // ランダム速度の下限
		uint32_t seed = 0;        // GPU乱数へ渡すシード値
		Vector3 maxVelocity{};    // ランダム速度の上限
		float minLifetime = 0.0f; // ランダム生存時間の下限
		Vector3 positionSpread{}; // 中心位置からの生成範囲
		float maxLifetime = 0.0f; // ランダム生存時間の上限
		Vector4 startColor{};     // 生成時の粒子色
		Vector4 endColor{};       // 消滅時の粒子色
		Vector2 startSize{};      // 生成時の粒子寸法
		Vector2 endSize{};        // 消滅時の粒子寸法
	};

	struct alignas(256) DrawConstants {
		Matrix4x4 viewProjection{}; // カメラのビュー・射影合成行列
		Vector3 cameraRight{};      // ビルボードの水平方向
		float padding0 = 0.0f;      // GPU定数バッファ配置を揃える余白
		Vector3 cameraUp{};         // ビルボードの垂直方向
		float padding1 = 0.0f;      // GPU定数バッファ配置を揃える余白
	};

	/// <summary>最大粒子数に合わせてGPUリソースを作成する。</summary>
	void CreateResources();

	/// <summary>粒子バッファへ指定したリソース状態遷移を記録する。</summary>
	void TransitionParticleBuffer(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	/// <summary>未使用粒子リストをGPU上で初期化する。</summary>
	void DispatchInitialize();

	/// <summary>現在フレームに蓄積した生成要求をGPUへ送る。</summary>
	void DispatchEmitCommands();

	/// <summary>全パーティクルのシミュレーションをGPUへ指示する。</summary>
	void DispatchUpdate();

	DirectXCommon *dxCommon_ = nullptr;             // DirectX基盤への非所有参照
	GPUParticlePipeline *pipeline_ = nullptr;       // Compute・描画パイプラインへの非所有参照
	TextureManager *textureManager_ = nullptr;      // 描画テクスチャ管理への非所有参照
	uint32_t textureHandle_ = 0;                    // 粒子描画に使用するテクスチャ番号
	uint32_t maxParticles_ = 0;                     // 同時に保持できる粒子数
	float deltaTime_ = 0.0f;                        // 最新更新で使用する経過時間
	bool needsInitialize_ = true;                   // GPU側の初期化処理が必要か
	bool simulationBuffersAreCommon_ = true;        // 補助バッファがCommon状態にあるか
	bool particleBufferIsSrv_ = false;              // 粒子バッファが描画用SRV状態にあるか
	GPUParticlePreset preset_{};                    // 現在の生成・更新設定
	std::vector<GPUParticleEmitData> pendingEmits_; // 次回更新で処理する生成要求

	Microsoft::WRL::ComPtr<ID3D12Resource> particleResource_;           // 全粒子データを保持するGPUバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> freeListResource_;           // 未使用粒子番号を保持するGPUバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> freeListIndexResource_;      // 未使用リスト末尾位置を保持するGPUバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> simulationConstantResource_; // 更新定数を保持するアップロードバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> emitConstantResource_;       // 生成定数を保持するアップロードバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> drawConstantResource_;       // 描画定数を保持するアップロードバッファ
	SimulationConstants *simulationConstants_ = nullptr;                // 更新定数のマップ済み領域
	EmitConstants *emitConstants_ = nullptr;                            // 生成定数のマップ済み領域
	DrawConstants *drawConstants_ = nullptr;                            // 描画定数のマップ済み領域
};
