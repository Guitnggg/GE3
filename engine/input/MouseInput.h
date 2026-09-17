#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <wrl.h>

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include <cstdint>

/// <summary>
/// DirectInputマウスの相対入力とクライアント座標を管理する。
/// </summary>
class MouseInput final {
public:
	MouseInput() = default;
	~MouseInput();
	MouseInput(const MouseInput&) = delete;
	MouseInput& operator=(const MouseInput&) = delete;

	/// <summary>
	/// 共有DirectInputからマウスデバイスを生成する。
	/// </summary>
	void Initialize(IDirectInput8* directInput, HWND hwnd);

	/// <summary>
	/// 相対入力とウィンドウ内の絶対カーソル座標を更新する。
	/// </summary>
	void Update();

	/// <summary>
	/// マウスの取得を解除して状態を消去する。
	/// </summary>
	void Finalize() noexcept;

public:
	/// <summary>
	/// 今フレームの水平方向の相対移動量を取得する。
	/// </summary>
	LONG GetDeltaX() const;

	/// <summary>
	/// 今フレームの垂直方向の相対移動量を取得する。
	/// </summary>
	LONG GetDeltaY() const;

	/// <summary>
	/// 今フレームのホイール回転量を取得する。
	/// </summary>
	LONG GetWheelDelta() const;

	/// <summary>
	/// ウィンドウのクライアント座標系でカーソル位置を取得する。
	/// </summary>
	POINT GetPosition() const;

	/// <summary>
	/// 指定ボタンが現在押されているか判定する。0は左ボタン。
	/// </summary>
	bool PushButton(uint32_t button) const;

	/// <summary>
	/// 指定ボタンがこのフレームに押されたか判定する。
	/// </summary>
	bool TriggerButton(uint32_t button) const;

	/// <summary>
	/// 指定ボタンがこのフレームに離されたか判定する。
	/// </summary>
	bool ReleaseButton(uint32_t button) const;

	/// <summary>
	/// マウス入力が初期化済みか判定する。
	/// </summary>
	bool IsInitialized() const { return initialized_; }

private:
	/// <summary>
	/// DIMOUSESTATE2が保持する8ボタンの範囲外アクセスを防ぐ。
	/// </summary>
	/// <param name="button">ボタン番号</param>
	void ValidateButton(uint32_t button) const;

	Microsoft::WRL::ComPtr<IDirectInputDevice8> device_; // DirectInputマウスデバイス
	DIMOUSESTATE2 current_{};                           // 今フレームの相対移動・ボタン状態
	DIMOUSESTATE2 previous_{};                          // 前フレームのボタン状態
	POINT position_{};                                  // クライアント座標系の絶対位置
	HWND hwnd_ = nullptr;                               // 座標変換に使う非所有ウィンドウハンドル
	bool initialized_ = false;
};
