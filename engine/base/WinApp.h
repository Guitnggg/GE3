#pragma once

#include <Windows.h>
#include <cstdint>

/// <summary>
/// Windowsアプリケーションのウィンドウ生成とメッセージ処理を管理するクラス
/// </summary>
class WinApp {
public:
	// ===== 静的メンバ関数 =====

	/// <summary>
	/// Windowsから送られるメッセージを処理するコールバック関数
	/// </summary>
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	// ===== メンバ関数 =====

	/// <summary>
	/// ウィンドウを生成し、Windowsアプリケーションを初期化する
	/// </summary>
	void Initialize();

	/// <summary>
	/// ウィンドウ関連の毎フレーム処理を行う
	/// </summary>
	void Update();

	/// <summary>
	/// ウィンドウとCOMを終了処理する
	/// </summary>
	void Finalize();

	/// <summary>
	/// ウィンドウハンドルを取得する
	/// </summary>
	HWND GetHwnd() const { return hwnd; }

	/// <summary>
	/// アプリケーションインスタンスを取得する
	/// </summary>
	HINSTANCE GetHInstance() const { return wc.hInstance; }

	/// <summary>
	/// Windowsメッセージを処理し、終了要求の有無を返す
	/// </summary>
	bool ProcessMessege();

public:
	// ===== 定数 =====

	static const int32_t kClientWidth = 1280;   // クライアント領域の横幅
	static const int32_t kClientHeight = 720;   // クライアント領域の縦幅

private:
	// ===== メンバ変数 =====

	HWND hwnd = nullptr;  // ウィンドウハンドル
	WNDCLASS wc{};        // ウィンドウクラス設定
};
