#pragma once

#include "engine/input/GamepadInput.h"
#include "engine/input/KeyboardInput.h"
#include "engine/input/MouseInput.h"

#include "engine/core/WinApp.h"

#include <string>
#include <unordered_map>
#include <vector>

using InputActionId = uint32_t;
inline constexpr InputActionId kInvalidInputActionId = 0;

enum class InputActionType {
	Button,
	Axis2D
};

/// <summary>
/// 各入力デバイスのライフサイクルをまとめ、従来互換の入力APIを提供する窓口。
/// デバイス固有機能はGetKeyboard/GetMouse/GetGamepadからも利用できる。
/// </summary>
class Input final {
  public:
	Input() = default;
	~Input();
	Input(const Input &) = delete;
	Input &operator=(const Input &) = delete;

	/// <summary>
	/// WinApp オブジェクトを使用してアプリケーションを初期化する。
	/// </summary>
	/// <param name="winApp">初期化に使用する WinApp インスタンスへのポインタ。</param>
	void Initialize(WinApp *winApp);

	/// <summary>
	/// 入力デバイスの状態を更新する。
	/// </summary>
	void Update();

	/// <summary>
	/// 入力デバイスを終了処理する。
	/// </summary>
	void Finalize() noexcept;

  public: // ===== キーボード入力の従来互換API =====
	bool PushKey(BYTE keyNumber) const {
		return keyboard_.Push(keyNumber);
	}
	bool TriggerKey(BYTE keyNumber) const {
		return keyboard_.Trigger(keyNumber);
	}
	bool ReleaseKey(BYTE keyNumber) const {
		return keyboard_.Release(keyNumber);
	}

  public: // ===== マウス入力の従来互換API =====
	LONG GetMouseDeltaX() const {
		return mouse_.GetDeltaX();
	}
	LONG GetMouseDeltaY() const {
		return mouse_.GetDeltaY();
	}
	LONG GetMouseWheelDelta() const {
		return mouse_.GetWheelDelta();
	}
	POINT GetMousePosition() const {
		return mouse_.GetPosition();
	}
	bool PushMouseButton(uint32_t button) const {
		return mouse_.PushButton(button);
	}
	bool TriggerMouseButton(uint32_t button) const {
		return mouse_.TriggerButton(button);
	}
	bool ReleaseMouseButton(uint32_t button) const {
		return mouse_.ReleaseButton(button);
	}

  public: // ===== ゲームパッド入力の従来互換API =====
	bool IsPadConnected() const {
		return gamepad_.IsConnected();
	}
	bool PushPadButton(WORD buttons) const {
		return gamepad_.PushButton(buttons);
	}
	bool TriggerPadButton(WORD buttons) const {
		return gamepad_.TriggerButton(buttons);
	}
	bool ReleasePadButton(WORD buttons) const {
		return gamepad_.ReleaseButton(buttons);
	}
	Vector2 GetLeftStick() const {
		return gamepad_.GetLeftStick();
	}
	Vector2 GetRightStick() const {
		return gamepad_.GetRightStick();
	}
	float GetLeftTrigger() const {
		return gamepad_.GetLeftTrigger();
	}
	float GetRightTrigger() const {
		return gamepad_.GetRightTrigger();
	}

  public: // ===== デバイス非依存のアクションAPI =====
	InputActionId AddButtonAction(const std::string &name);
	InputActionId AddAxis2DAction(const std::string &name);
	void BindKey(InputActionId action, BYTE key);
	void BindMouseButton(InputActionId action, uint32_t button);
	void BindPadButton(InputActionId action, WORD button);
	void BindKeyboardAxis2D(InputActionId action, BYTE up, BYTE down, BYTE left, BYTE right);
	void BindLeftStick(InputActionId action, float deadZone = 0.0f);

	bool PushAction(InputActionId action) const;
	bool TriggerAction(InputActionId action) const;
	bool ReleaseAction(InputActionId action) const;
	Vector2 GetActionAxis2D(InputActionId action) const;
	InputActionId FindAction(const std::string &name) const;

  public: // ===== デバイス固有機能の取得 =====
	KeyboardInput &GetKeyboard() {
		return keyboard_;
	}
	const KeyboardInput &GetKeyboard() const {
		return keyboard_;
	}
	MouseInput &GetMouse() {
		return mouse_;
	}
	const MouseInput &GetMouse() const {
		return mouse_;
	}
	GamepadInput &GetGamepad() {
		return gamepad_;
	}
	const GamepadInput &GetGamepad() const {
		return gamepad_;
	}
	bool IsInitialized() const {
		return initialized_;
	}

  private:
	enum class BindingDevice {
		Keyboard,
		Mouse,
		Gamepad
	};
	struct ButtonBinding {
		BindingDevice device = BindingDevice::Keyboard;
		uint32_t code = 0;
		bool operator==(const ButtonBinding &) const = default;
	};
	struct KeyboardAxis2DBinding {
		BYTE up = 0;
		BYTE down = 0;
		BYTE left = 0;
		BYTE right = 0;
		bool operator==(const KeyboardAxis2DBinding &) const = default;
	};
	struct InputAction {
		std::string name;
		InputActionType type = InputActionType::Button;
		std::vector<ButtonBinding> buttonBindings;
		std::vector<KeyboardAxis2DBinding> keyboardAxis2DBindings;
		bool useLeftStick = false;
		float stickDeadZone = 0.0f;
		bool previousPressed = false;
		bool pressed = false;
		Vector2 axis2D{};
	};

	InputActionId AddAction(const std::string &name, InputActionType type);
	InputAction &RequireAction(InputActionId id, InputActionType expectedType);
	const InputAction &RequireAction(InputActionId id, InputActionType expectedType) const;
	void UpdateActions();
	bool IsBindingPressed(const ButtonBinding &binding) const;

	Microsoft::WRL::ComPtr<IDirectInput8> directInput_;
	KeyboardInput keyboard_;
	MouseInput mouse_;
	GamepadInput gamepad_;
	WinApp *winApp_ = nullptr;
	std::vector<InputAction> actions_;
	std::unordered_map<std::string, InputActionId> actionIds_;
	bool initialized_ = false;
};
