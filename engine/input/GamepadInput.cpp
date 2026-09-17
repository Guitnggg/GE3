#include "GamepadInput.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#pragma comment(lib,"xinput.lib")

GamepadInput::~GamepadInput() {
	Finalize();
}

void GamepadInput::Initialize() {
	// XInputが対応するユーザー番号は0～3のみ
	if (initialized_) { throw std::logic_error("GamepadInput is already initialized."); }
	if (userIndex_ >= XUSER_MAX_COUNT) { throw std::out_of_range("Invalid XInput user index."); }
	current_ = {};
	previous_ = {};
	connected_ = false;
	initialized_ = true;
	// 起動時点の接続状態もすぐ参照できるよう、初回状態を取得する
	Update();
	previous_ = current_; // 初期化時に押されていたボタンをトリガー扱いしない
}

void GamepadInput::Update() {
	if (!initialized_) { throw std::logic_error("GamepadInput is not initialized."); }
	// 切断された場合も前回状態を残してから現在状態をゼロにし、Release判定を可能にする
	previous_ = current_;
	XINPUT_STATE next{};
	connected_ = XInputGetState(userIndex_, &next) == ERROR_SUCCESS;
	current_ = connected_ ? next : XINPUT_STATE{};
}

void GamepadInput::Finalize() noexcept {
	current_ = {};
	previous_ = {};
	connected_ = false;
	initialized_ = false;
}

void GamepadInput::ValidateButtons(WORD buttons) const {
	// 0は常に成立してしまうため、入力条件として受け付けない
	if (!initialized_) { throw std::logic_error("GamepadInput is not initialized."); }
	if (buttons == 0) { throw std::invalid_argument("Pad button mask must not be zero."); }
}
bool GamepadInput::PushButton(WORD buttons) const {
	ValidateButtons(buttons);
	return connected_ && (current_.Gamepad.wButtons & buttons) == buttons;
}
bool GamepadInput::TriggerButton(WORD buttons) const {
	ValidateButtons(buttons);
	return connected_ && (current_.Gamepad.wButtons & buttons) == buttons &&
		(previous_.Gamepad.wButtons & buttons) != buttons;
}
bool GamepadInput::ReleaseButton(WORD buttons) const {
	ValidateButtons(buttons);
	return (current_.Gamepad.wButtons & buttons) != buttons &&
		(previous_.Gamepad.wButtons & buttons) == buttons;
}

Vector2 GamepadInput::NormalizeStick(SHORT rawX, SHORT rawY, SHORT deadZone) {
	// 軸ごとではなく円形デッドゾーンを使い、斜め方向の感度差を防ぐ
	const float x = static_cast<float>(rawX);
	const float y = static_cast<float>(rawY);
	const float magnitude = std::sqrt(x * x + y * y);
	if (magnitude <= static_cast<float>(deadZone)) { return {}; }
	constexpr float kMaximum = 32767.0f;
	const float legalMagnitude = std::min(magnitude, kMaximum);
	// デッドゾーンの外側だけを0～1へ再マッピングする
	const float normalized = (legalMagnitude - deadZone) / (kMaximum - deadZone);
	return {x / magnitude * normalized, y / magnitude * normalized};
}

float GamepadInput::NormalizeTrigger(BYTE value) {
	// XInput標準の閾値以下をノイズとして除去し、残りの範囲を0～1へ変換する
	if (value <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) { return 0.0f; }
	return static_cast<float>(value - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) /
		static_cast<float>(255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
}

Vector2 GamepadInput::GetLeftStick() const {
	if (!initialized_) { throw std::logic_error("GamepadInput is not initialized."); }
	return connected_ ? NormalizeStick(current_.Gamepad.sThumbLX, current_.Gamepad.sThumbLY,
		XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) : Vector2{};
}
Vector2 GamepadInput::GetRightStick() const {
	if (!initialized_) { throw std::logic_error("GamepadInput is not initialized."); }
	return connected_ ? NormalizeStick(current_.Gamepad.sThumbRX, current_.Gamepad.sThumbRY,
		XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) : Vector2{};
}
float GamepadInput::GetLeftTrigger() const {
	if (!initialized_) { throw std::logic_error("GamepadInput is not initialized."); }
	return connected_ ? NormalizeTrigger(current_.Gamepad.bLeftTrigger) : 0.0f;
}
float GamepadInput::GetRightTrigger() const {
	if (!initialized_) { throw std::logic_error("GamepadInput is not initialized."); }
	return connected_ ? NormalizeTrigger(current_.Gamepad.bRightTrigger) : 0.0f;
}
