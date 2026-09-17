#include "MouseInput.h"

#include "engine/core/diagnostics/HResult.h"

#include <iterator>
#include <stdexcept>

MouseInput::~MouseInput() {
	Finalize();
}

void MouseInput::Initialize(IDirectInput8* directInput, HWND hwnd) {
	if (initialized_ || device_ || hwnd_ != nullptr) { throw std::logic_error("MouseInput is already initialized."); }
	if (directInput == nullptr || hwnd == nullptr) {
		throw std::invalid_argument("MouseInput requires DirectInput and a window handle.");
	}
	hwnd_ = hwnd;
	try {
		// DIMOUSESTATE2を使い、移動・ホイール・最大8ボタンをまとめて取得する
		HRESULT result = directInput->CreateDevice(GUID_SysMouse, device_.ReleaseAndGetAddressOf(), nullptr);
		HResult::ThrowIfFailed(result, "Creating the mouse input device");
		result = device_->SetDataFormat(&c_dfDIMouse2);
		HResult::ThrowIfFailed(result, "Setting the mouse data format");
		result = device_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		HResult::ThrowIfFailed(result, "Setting the mouse cooperative level");
		current_ = {};
		previous_ = {};
		position_ = {};
		initialized_ = true;
	} catch (...) {
		Finalize();
		throw;
	}
}

void MouseInput::Update() {
	if (!initialized_ || !device_) { throw std::logic_error("MouseInput is not initialized."); }
	// ボタンのトリガー／リリース判定用に前回状態を保存する
	previous_ = current_;
	HRESULT result = device_->GetDeviceState(sizeof(current_), &current_);
	if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
		// フォーカス復帰時はデバイスを再取得して入力を読み直す
		result = device_->Acquire();
		if (SUCCEEDED(result)) { result = device_->GetDeviceState(sizeof(current_), &current_); }
	}
	// 入力権がない間は古いボタン状態や移動量を残さない
	if (FAILED(result)) { current_ = {}; }

	// GetCursorPosは画面全体の座標なので、描画と同じクライアント座標へ変換する
	POINT cursor{};
	if (GetCursorPos(&cursor) && ScreenToClient(hwnd_, &cursor)) { position_ = cursor; }
}

void MouseInput::Finalize() noexcept {
	if (device_) { device_->Unacquire(); }
	device_.Reset();
	current_ = {};
	previous_ = {};
	position_ = {};
	hwnd_ = nullptr;
	initialized_ = false;
}

LONG MouseInput::GetDeltaX() const {
	if (!initialized_) { throw std::logic_error("MouseInput is not initialized."); }
	return current_.lX;
}
LONG MouseInput::GetDeltaY() const {
	if (!initialized_) { throw std::logic_error("MouseInput is not initialized."); }
	return current_.lY;
}
LONG MouseInput::GetWheelDelta() const {
	if (!initialized_) { throw std::logic_error("MouseInput is not initialized."); }
	return current_.lZ;
}
POINT MouseInput::GetPosition() const {
	if (!initialized_) { throw std::logic_error("MouseInput is not initialized."); }
	return position_;
}

void MouseInput::ValidateButton(uint32_t button) const {
	// DIMOUSESTATE2が保持する8ボタンの範囲外アクセスを防ぐ
	if (!initialized_) { throw std::logic_error("MouseInput is not initialized."); }
	if (button >= std::size(current_.rgbButtons)) { throw std::out_of_range("Invalid mouse button index."); }
}
bool MouseInput::PushButton(uint32_t button) const {
	ValidateButton(button);
	return (current_.rgbButtons[button] & 0x80) != 0;
}
bool MouseInput::TriggerButton(uint32_t button) const {
	ValidateButton(button);
	return (current_.rgbButtons[button] & 0x80) != 0 && (previous_.rgbButtons[button] & 0x80) == 0;
}
bool MouseInput::ReleaseButton(uint32_t button) const {
	ValidateButton(button);
	return (current_.rgbButtons[button] & 0x80) == 0 && (previous_.rgbButtons[button] & 0x80) != 0;
}
