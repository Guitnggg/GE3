#pragma once

#include <Windows.h>
#include <wrl.h>

#define DIRECTINPUT_VERSION 0x0800  // 使用するDirectInputのバージョン
#include <dinput.h>

#include "WinApp.h"

/// <summary>
/// キーボード入力を管理するクラス
/// </summary>
class Input {
public:
	// ===== 型エイリアス =====

	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// ===== メンバ関数 =====

	/// <summary>
	/// DirectInputとキーボードデバイスを初期化する
	/// </summary>
	/// <param name="winApp">Windowsアプリケーション</param>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// キーボード入力状態を更新する
	/// </summary>
	void Update();

	/// <summary>
	/// 指定キーが押されているかを取得する
	/// </summary>
	/// <param name="keyNumber">DirectInputのキー番号</param>
	/// <returns>押されていればtrue</returns>
	bool PushKey(BYTE keyNumber);

	/// <summary>
	/// 指定キーがトリガー入力されたかを取得する
	/// </summary>
	/// <param name="keyNumber">DirectInputのキー番号</param>
	/// <returns>トリガー入力されていればtrue</returns>
	bool TriggerKey(BYTE keyNumber);

private:
	// ===== DirectInput関連 =====

	ComPtr<IDirectInputDevice8> keyboard;  // キーボードデバイス
	ComPtr<IDirectInput8> directInput;     // DirectInput本体

	// ===== キー入力状態 =====

	BYTE key[256] = {};     // 現在の全キー状態
	BYTE keyPre[256] = {};  // 前回の全キー状態

	WinApp* winApp = nullptr;  // Windowsアプリケーション
};
