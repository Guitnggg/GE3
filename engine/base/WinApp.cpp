#include "WinApp.h"

#include "externals/imgui/imgui.h"
#include <stdexcept>

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
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to initialize COM.");
	}

	// ウィンドウクラスを設定する
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"C62WindowClass";
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	if (RegisterClass(&wc) == 0) {
		throw std::runtime_error("Failed to register the window class.");
	}

	// クライアント領域のサイズから実際のウィンドウサイズを計算する
	constexpr DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	RECT wrc = { 0, 0,kClientWidth,kClientHeight };
	AdjustWindowRect(&wrc, windowStyle, false);

	// ウィンドウを生成する
		hwnd = CreateWindow(
		wc.lpszClassName,
		L"CG2",
		windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
			nullptr);
	if (hwnd == nullptr) {
		throw std::runtime_error("Failed to create the application window.");
	}

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
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			return true;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return false;
}
