#include "WinApp.h"

#include "externals/imgui/imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// Windowsメッセージに応じてアプリ固有の処理を行う
	switch (msg) {
	case WM_DESTROY:
		// OSへアプリ終了を通知する
		PostQuitMessage(0);
		return 0;
	}

	// ImGuiが使用するメッセージを先に処理する
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	// 標準のWindowsメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::Initialize()
{
	// COMライブラリをマルチスレッドで初期化する
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	// ウィンドウクラスを設定する
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"C62WindowClass";
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc);

	// クライアント領域のサイズから実際のウィンドウサイズを計算する
	RECT wrc = { 0, 0,kClientWidth,kClientHeight };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウを生成する
	hwnd = CreateWindow(
		wc.lpszClassName,
		L"CG2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr);

	// 生成したウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);
}

void WinApp::Update()
{
}

void WinApp::Finalize()
{
	// ウィンドウとCOMライブラリを終了する
	CloseWindow(hwnd);
	CoUninitialize();
}

bool WinApp::ProcessMessege()
{
	MSG msg{};

	// キューにあるWindowsメッセージを処理する
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// WM_QUITを受け取ったらアプリを終了する
	if (msg.message == WM_QUIT) {
		return true;
	}

	return false;
}
