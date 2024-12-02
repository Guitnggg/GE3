#pragma once

#include <Windows.h>
#include <cstdint>

/// <summary>
/// WindowsAPI
/// </summary>

class WinApp
{
public:
	//====================
	// 静的メンバ関数
	//====================

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	//====================
	// メンバ関数
	//====================

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 終了
	void Finalize();

	// getter
	HWND GetHwnd()const { return hwnd; }

	HINSTANCE GetHInstance()const { return wc.hInstance; }


public:
	//====================
	// 定数
	//====================

	//クライアント領域のサイズ　横　縦
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

private:
	//====================
	// メンバ変数
	//====================

	// ウィンドウハンドル
	HWND hwnd = nullptr;

	// ウィンドウクラスの設定
	WNDCLASS wc{};
};
