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

/// <summary>
/// DirectInputキーボードの状態とフレーム間変化を管理する。
/// </summary>
class KeyboardInput final {
public:
	KeyboardInput() = default;
	~KeyboardInput();
	KeyboardInput(const KeyboardInput&) = delete;
	KeyboardInput& operator=(const KeyboardInput&) = delete;

	/// <summary>
	/// 共有DirectInputからキーボードデバイスを生成する。
	/// </summary>
	void Initialize(IDirectInput8* directInput, HWND hwnd);

	/// <summary>
	/// 現在状態を前回状態へ退避してから最新の256キー状態を取得する。
	/// </summary>
	void Update();

	/// <summary>
	/// キーボードの取得を解除して状態を消去する。
	/// </summary>
	void Finalize() noexcept;

public:
	/// <summary>
	/// 指定キーが現在押されているか判定する。
	/// </summary>
	bool Push(BYTE keyNumber) const;

	/// <summary>
	/// 指定キーがこのフレームに押されたか判定する。
	/// </summary>
	bool Trigger(BYTE keyNumber) const;

	/// <summary>
	/// 指定キーがこのフレームに離されたか判定する。
	/// </summary>
	bool Release(BYTE keyNumber) const;

	/// <summary>
	/// キーボード入力が初期化済みか判定する。
	/// </summary>
	bool IsInitialized() const { return initialized_; }

private:
	Microsoft::WRL::ComPtr<IDirectInputDevice8> device_; // DirectInputキーボードデバイス
	BYTE current_[256]{};                               // 今フレームの全キー状態
	BYTE previous_[256]{};                              // 前フレームの全キー状態
	bool initialized_ = false;
};
