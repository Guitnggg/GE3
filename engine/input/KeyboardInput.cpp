#include "KeyboardInput.h"

#include "engine/core/diagnostics/HResult.h"

#include <cstring>
#include <stdexcept>

KeyboardInput::~KeyboardInput() {
	Finalize();
}

void KeyboardInput::Initialize(IDirectInput8* directInput, HWND hwnd) {
	if (initialized_ || device_) { throw std::logic_error("KeyboardInput is already initialized."); }
	if (directInput == nullptr || hwnd == nullptr) {
		throw std::invalid_argument("KeyboardInput requires DirectInput and a window handle.");
	}
	try {
		// 前面ウィンドウの間だけ非排他で取得し、他アプリのキー入力を妨げない
		HRESULT result = directInput->CreateDevice(GUID_SysKeyboard, device_.ReleaseAndGetAddressOf(), nullptr);
		HResult::ThrowIfFailed(result, "Creating the keyboard input device");
		result = device_->SetDataFormat(&c_dfDIKeyboard);
		HResult::ThrowIfFailed(result, "Setting the keyboard data format");
		result = device_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
		HResult::ThrowIfFailed(result, "Setting the keyboard cooperative level");
		std::memset(current_, 0, sizeof(current_));
		std::memset(previous_, 0, sizeof(previous_));
		initialized_ = true;
	} catch (...) {
		Finalize();
		throw;
	}
}

void KeyboardInput::Update() {
	if (!initialized_ || !device_) { throw std::logic_error("KeyboardInput is not initialized."); }
	// 差分判定用に前回状態を保存してから最新状態を取得する
	std::memcpy(previous_, current_, sizeof(current_));
	HRESULT result = device_->GetDeviceState(sizeof(current_), current_);
	if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
		// Alt+Tabなどで入力権を失った場合は再取得後に一度だけ読み直す
		result = device_->Acquire();
		if (SUCCEEDED(result)) { result = device_->GetDeviceState(sizeof(current_), current_); }
	}
	// 取得失敗時に押下状態を残すとキーが押しっぱなしになるためゼロへ戻す
	if (FAILED(result)) { std::memset(current_, 0, sizeof(current_)); }
}

void KeyboardInput::Finalize() noexcept {
	if (device_) { device_->Unacquire(); }
	device_.Reset();
	std::memset(current_, 0, sizeof(current_));
	std::memset(previous_, 0, sizeof(previous_));
	initialized_ = false;
}

bool KeyboardInput::Push(BYTE keyNumber) const {
	if (!initialized_) { throw std::logic_error("KeyboardInput is not initialized."); }
	return (current_[keyNumber] & 0x80) != 0;
}

bool KeyboardInput::Trigger(BYTE keyNumber) const {
	if (!initialized_) { throw std::logic_error("KeyboardInput is not initialized."); }
	return (current_[keyNumber] & 0x80) != 0 && (previous_[keyNumber] & 0x80) == 0;
}

bool KeyboardInput::Release(BYTE keyNumber) const {
	if (!initialized_) { throw std::logic_error("KeyboardInput is not initialized."); }
	return (current_[keyNumber] & 0x80) == 0 && (previous_[keyNumber] & 0x80) != 0;
}
