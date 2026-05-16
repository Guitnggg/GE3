#pragma once

#include <Windows.h>
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>

#include "WinApp.h"

/// <summary>
/// 入力
/// </summary>

class Input
{
public: // メンバ関数

	// namespace省略
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// 初期化
	void Initialize(WinApp* winApp);

	// 更新
	void Update();

	// 押されているか
	bool PushKey(BYTE keyNumber);

	// トリガー
	bool TriggerKey(BYTE keyNumber);

private: // メンバ変数

	// キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard;

	// DirectInputのインスタンス
	ComPtr<IDirectInput8>directInput;

	// 全キーの状態
	BYTE key[256] = {};

	// 前回の全キーの状態
	BYTE keyPre[256] = {};

	// WindowsAPI
	WinApp* winApp = nullptr;
};

