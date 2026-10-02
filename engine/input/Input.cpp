#include "Input.h"

#include "engine/core/diagnostics/HResult.h"

#include <algorithm>
#include <cmath>
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
	UpdateActions();
}

void Input::Finalize() noexcept {
	// 各デバイスを先に終了してから、それらが参照するDirectInput本体を解放する
	gamepad_.Finalize();
	mouse_.Finalize();
	keyboard_.Finalize();
	directInput_.Reset();
	actions_.clear();
	actionIds_.clear();
	winApp_ = nullptr;
	initialized_ = false;
}

InputActionId Input::AddAction(const std::string& name, InputActionType type) {
	if (name.empty()) { throw std::invalid_argument("Input action name must not be empty."); }
	if (const auto found = actionIds_.find(name); found != actionIds_.end()) {
		const InputAction& action = actions_.at(found->second - 1);
		if (action.type != type) { throw std::invalid_argument("Input action already exists with a different type: " + name); }
		return found->second;
	}
	const InputActionId id = static_cast<InputActionId>(actions_.size() + 1);
	actions_.push_back(InputAction{name, type});
	actionIds_.emplace(name, id);
	return id;
}

InputActionId Input::AddButtonAction(const std::string& name) { return AddAction(name, InputActionType::Button); }
InputActionId Input::AddAxis2DAction(const std::string& name) { return AddAction(name, InputActionType::Axis2D); }

Input::InputAction& Input::RequireAction(InputActionId id, InputActionType expectedType) {
	if (id == kInvalidInputActionId || id > actions_.size()) { throw std::out_of_range("Invalid input action ID."); }
	InputAction& action = actions_[id - 1];
	if (action.type != expectedType) { throw std::invalid_argument("Input action type does not match the requested operation."); }
	return action;
}

const Input::InputAction& Input::RequireAction(InputActionId id, InputActionType expectedType) const {
	if (id == kInvalidInputActionId || id > actions_.size()) { throw std::out_of_range("Invalid input action ID."); }
	const InputAction& action = actions_[id - 1];
	if (action.type != expectedType) { throw std::invalid_argument("Input action type does not match the requested operation."); }
	return action;
}

void Input::BindKey(InputActionId action, BYTE key) {
	auto& bindings = RequireAction(action, InputActionType::Button).buttonBindings;
	const ButtonBinding binding{BindingDevice::Keyboard, key};
	if (std::find(bindings.begin(), bindings.end(), binding) == bindings.end()) { bindings.push_back(binding); }
}

void Input::BindMouseButton(InputActionId action, uint32_t button) {
	if (button >= 8) { throw std::out_of_range("Mouse button must be in the range 0-7."); }
	auto& bindings = RequireAction(action, InputActionType::Button).buttonBindings;
	const ButtonBinding binding{BindingDevice::Mouse, button};
	if (std::find(bindings.begin(), bindings.end(), binding) == bindings.end()) { bindings.push_back(binding); }
}

void Input::BindPadButton(InputActionId action, WORD button) {
	if (button == 0) { throw std::invalid_argument("Gamepad button mask must not be zero."); }
	auto& bindings = RequireAction(action, InputActionType::Button).buttonBindings;
	const ButtonBinding binding{BindingDevice::Gamepad, button};
	if (std::find(bindings.begin(), bindings.end(), binding) == bindings.end()) { bindings.push_back(binding); }
}

void Input::BindKeyboardAxis2D(InputActionId action, BYTE up, BYTE down, BYTE left, BYTE right) {
	auto& bindings = RequireAction(action, InputActionType::Axis2D).keyboardAxis2DBindings;
	const KeyboardAxis2DBinding binding{up, down, left, right};
	if (std::find(bindings.begin(), bindings.end(), binding) == bindings.end()) { bindings.push_back(binding); }
}

void Input::BindLeftStick(InputActionId action, float deadZone) {
	if (!std::isfinite(deadZone) || deadZone < 0.0f || deadZone >= 1.0f) {
		throw std::invalid_argument("Stick dead zone must be finite and in the range [0, 1).");
	}
	InputAction& inputAction = RequireAction(action, InputActionType::Axis2D);
	inputAction.useLeftStick = true;
	inputAction.stickDeadZone = deadZone;
}

bool Input::IsBindingPressed(const ButtonBinding& binding) const {
	switch (binding.device) {
	case BindingDevice::Keyboard: return keyboard_.Push(static_cast<BYTE>(binding.code));
	case BindingDevice::Mouse: return mouse_.PushButton(binding.code);
	case BindingDevice::Gamepad: return gamepad_.PushButton(static_cast<WORD>(binding.code));
	}
	return false;
}

void Input::UpdateActions() {
	for (InputAction& action : actions_) {
		if (action.type == InputActionType::Button) {
			action.previousPressed = action.pressed;
			action.pressed = std::any_of(action.buttonBindings.begin(), action.buttonBindings.end(),
				[this](const ButtonBinding& binding) { return IsBindingPressed(binding); });
			continue;
		}
		Vector2 value{};
		for (const KeyboardAxis2DBinding& binding : action.keyboardAxis2DBindings) {
			if (keyboard_.Push(binding.left)) { value.x -= 1.0f; }
			if (keyboard_.Push(binding.right)) { value.x += 1.0f; }
			if (keyboard_.Push(binding.up)) { value.y += 1.0f; }
			if (keyboard_.Push(binding.down)) { value.y -= 1.0f; }
		}
		if (action.useLeftStick) {
			Vector2 stick = gamepad_.GetLeftStick();
			const float magnitude = std::sqrt(stick.x * stick.x + stick.y * stick.y);
			if (magnitude > action.stickDeadZone) {
				const float remapped = (magnitude - action.stickDeadZone) / (1.0f - action.stickDeadZone);
				value.x += stick.x / magnitude * remapped;
				value.y += stick.y / magnitude * remapped;
			}
		}
		const float length = std::sqrt(value.x * value.x + value.y * value.y);
		if (length > 1.0f) { value.x /= length; value.y /= length; }
		action.axis2D = value;
	}
}

bool Input::PushAction(InputActionId action) const {
	return RequireAction(action, InputActionType::Button).pressed;
}

bool Input::TriggerAction(InputActionId action) const {
	const InputAction& state = RequireAction(action, InputActionType::Button);
	return state.pressed && !state.previousPressed;
}

bool Input::ReleaseAction(InputActionId action) const {
	const InputAction& state = RequireAction(action, InputActionType::Button);
	return !state.pressed && state.previousPressed;
}

Vector2 Input::GetActionAxis2D(InputActionId action) const {
	return RequireAction(action, InputActionType::Axis2D).axis2D;
}

InputActionId Input::FindAction(const std::string& name) const {
	const auto found = actionIds_.find(name);
	return found == actionIds_.end() ? kInvalidInputActionId : found->second;
}
