#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <Xinput.h>

#include "engine/math/Mymath.h"

/// <summary>
/// 1台のXInputゲームパッドの状態とフレーム間変化を管理する。
/// </summary>
class GamepadInput final {
public:
	/// <param name="userIndex">XInputのユーザー番号（0～3）</param>
	explicit GamepadInput(DWORD userIndex = 0) : userIndex_(userIndex) {}
	~GamepadInput();
	GamepadInput(const GamepadInput&) = delete;
	GamepadInput& operator=(const GamepadInput&) = delete;

	/// <summary>
	/// 指定ユーザー番号のゲームパッド状態を初期化する。
	/// </summary>
	void Initialize();

	/// <summary>
	/// 接続状態と現在のXInput状態を更新する。
	/// </summary>
	void Update();

	/// <summary>
	/// 保持している入力状態を消去する。
	/// </summary>
	void Finalize() noexcept;

public:
	/// <summary>
	/// ゲームパッドが現在接続されているか取得する。
	/// </summary>
	bool IsConnected() const { return connected_; }

	/// <summary>
	/// 指定したボタンマスクがすべて押されているか判定する。
	/// </summary>
	bool PushButton(WORD buttons) const;

	/// <summary>
	/// 指定したボタンマスクがこのフレームに成立したか判定する。
	/// </summary>
	bool TriggerButton(WORD buttons) const;

	/// <summary>
	/// 指定したボタンマスクがこのフレームに解除されたか判定する。
	/// </summary>
	bool ReleaseButton(WORD buttons) const;

	/// <summary>
	/// デッドゾーン適用済みの左スティック値を-1～1で取得する。
	/// </summary>
	Vector2 GetLeftStick() const;

	/// <summary>
	/// デッドゾーン適用済みの右スティック値を-1～1で取得する。
	/// </summary>
	Vector2 GetRightStick() const;

	/// <summary>
	/// 閾値適用済みの左トリガー値を0～1で取得する。
	/// </summary>
	float GetLeftTrigger() const;

	/// <summary>
	/// 閾値適用済みの右トリガー値を0～1で取得する。
	/// </summary>
	float GetRightTrigger() const;

	/// <summary>
	/// ゲームパッドが初期化されているか取得する。
	/// </summary>
	/// <returns>初期化されていればtrue、それ以外はfalse</returns>
	bool IsInitialized() const { return initialized_; }

	/// <summary>
	/// XInputが識別するユーザー番号（0～3）を取得する。
	/// </summary>
	/// <returns>ユーザー番号（0～3）</returns>
	DWORD GetUserIndex() const { return userIndex_; }

private:
	/// <summary>
	/// 指定したボタンマスクが有効か検証する。
	/// </summary>
	/// <param name="buttons">ボタンマスク</param>
	void ValidateButtons(WORD buttons) const;

	/// <summary>
	/// スティックの2次元入力 (x, y) をデッドゾーンを考慮して正規化し、2Dベクトルを返します。
	/// </summary>
	/// <param name="x">スティックの X 軸入力（SHORT 型）。</param>
	/// <param name="y">スティックの Y 軸入力（SHORT 型）。</param>
	/// <param name="deadZone">デッドゾーンの閾値（同じ単位／スケールの SHORT）。入力の大きさがこの値未満の場合、(0,0) を返すために使用します。</param>
	/// <returns>正規化された Vector2（単位ベクトル）。入力がデッドゾーン内の場合はゼロベクトルを返します。</returns>
	static Vector2 NormalizeStick(SHORT x, SHORT y, SHORT deadZone);

	/// <summary>
	/// トリガーの入力値を閾値を考慮して正規化し、0～1の範囲に変換します。
	/// </summary>
	/// <param name="value">トリガーの入力値（BYTE 型）。</param>
	/// <returns>正規化されたトリガー値（0～1の範囲）。</returns>
	static float NormalizeTrigger(BYTE value);

private:
	XINPUT_STATE current_{};  // 今フレームのボタン・スティック・トリガー状態
	XINPUT_STATE previous_{}; // 前フレームの状態
	DWORD userIndex_ = 0;     // XInputが識別する0～3のユーザー番号
	bool connected_ = false;  // 最後の状態取得に成功したか
	bool initialized_ = false;
};
