#include "Input.h"

#include "engine/core/diagnostics/HResult.h"

#include <stdexcept>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

Input::~Input() {
	Finalize();
}

void Input::Initialize(WinApp* winApp) {
	// 一部だけ残った状態も拒否し、同じデバイスを二重生成しない
	if (initialized_ || directInput_ || winApp_ != nullptr || keyboard_.IsInitialized() ||
		mouse_.IsInitialized() || gamepad_.IsInitialized()) {
		throw std::logic_error("Input is already initialized or partially initialized.");
	}
	if (winApp == nullptr || winApp->GetHwnd() == nullptr || winApp->GetHInstance() == nullptr) {
		throw std::invalid_argument("Input requires an initialized WinApp instance.");
	}

	winApp_ = winApp;
	try {
		// DirectInput本体はキーボードとマウスで共有し、XInputパッドは独立して初期化する
		const HRESULT result = DirectInput8Create(
			winApp_->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
			reinterpret_cast<void**>(directInput_.ReleaseAndGetAddressOf()), nullptr);
		HResult::ThrowIfFailed(result, "Initializing DirectInput");
		keyboard_.Initialize(directInput_.Get(), winApp_->GetHwnd());
		mouse_.Initialize(directInput_.Get(), winApp_->GetHwnd());
		gamepad_.Initialize();
		initialized_ = true;
	} catch (...) {
		// 途中のデバイス生成に失敗しても、生成済みのものを依存順に破棄する
		Finalize();
		throw;
	}
}

void Input::Update() {
	if (!initialized_) { throw std::logic_error("Input is not initialized."); }
	// ゲーム側からは1回の呼び出しで全入力のフレーム状態を確定できる
	keyboard_.Update();
	mouse_.Update();
	gamepad_.Update();
}

void Input::Finalize() noexcept {
	// 各デバイスを先に終了してから、それらが参照するDirectInput本体を解放する
	gamepad_.Finalize();
	mouse_.Finalize();
	keyboard_.Finalize();
	directInput_.Reset();
	winApp_ = nullptr;
	initialized_ = false;
}
