#pragma once

#include <cstdint>

/// <summary>
/// ゲームプレイ中に調整できるパラメータ一式。
/// </summary>
struct GameParameters {
	float railSpeed = 8.0f;                    // カメラがレール上を前進する速度
	float playerMoveSpeed = 5.5f;              // WASDによるプレイヤー機体の移動速度
	float enemySpawnInterval = 0.85f;          // 得点補正前の敵出現間隔（秒）
	float minimumSpawnInterval = 0.38f;        // 難易度上昇後も下回らない出現間隔（秒）
	float spawnAccelerationPerScore = 0.012f;  // 1得点ごとに短縮する出現間隔（秒）
	float enemySpawnDistance = 62.0f;          // カメラから敵を生成する位置までの距離
	float enemyBaseRadius = 0.8f;              // 敵の基本半径
	float enemyRadiusStep = 0.14f;             // 敵ごとに加算する半径の変化量
	float enemyRotationSpeed = 1.5f;           // 敵のY軸回転速度（ラジアン/秒）
	uint32_t startingLives = 3;                // リスタート時に設定する初期ライフ
};

/// <summary>
/// GameParametersを実行中に編集するデバッグUI。
/// </summary>
class GameParameterEditor final {
public:
	/// <summary>
	/// パラメータ編集、一時停止、再スタート操作を描画する。
	/// </summary>
	/// <param name="parameters">編集対象のゲームパラメータ</param>
	/// <returns>ゲームの再スタートが要求された場合はtrue</returns>
	bool Draw(GameParameters& parameters);

	/// <summary>
	/// エディタからゲームが一時停止されているか取得する。
	/// </summary>
	bool IsPaused() const { return paused_; }

	/// <summary>
	/// 一時停止状態を外部から設定する。
	/// </summary>
	void SetPaused(bool paused) { paused_ = paused; }

private:
	bool paused_ = false; // ゲームロジックを停止するか
};
