#pragma once

#include <Windows.h>
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>

/// <summary>
/// 入力
/// </summary>

class Input
{
public: // メンバ関数

	// namespace省略
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// 初期化
	void Initialize(HINSTANCE hInstance, HWND hwnd);

	// 更新
	void Update();

private: // メンバ変数

	// キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard;

};

