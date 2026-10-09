#pragma once

#include "engine/input/GamepadInput.h"
#include "engine/input/KeyboardInput.h"
#include "engine/input/MouseInput.h"

#include "engine/core/WinApp.h"

#include <string>
#include <unordered_map>
#include <vector>

using InputActionId = uint32_t;
inline constexpr InputActionId kInvalidInputActionId = 0; // 無効な入力アクションを表す予約番号

enum class InputActionType {
	Button, // 押下状態を扱うデジタル入力
	Axis2D  // 二次元ベクトルを扱う方向入力
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
	/// <summary>指定したキーが押されているかを返す。</summary>
	bool PushKey(BYTE keyNumber) const {
		return keyboard_.Push(keyNumber);
	}

	/// <summary>指定したキーがこのフレームで押されたかを返す。</summary>
	bool TriggerKey(BYTE keyNumber) const {
		return keyboard_.Trigger(keyNumber);
	}

	/// <summary>指定したキーがこのフレームで離されたかを返す。</summary>
	bool ReleaseKey(BYTE keyNumber) const {
		return keyboard_.Release(keyNumber);
	}

  public: // ===== マウス入力の従来互換API =====
	/// <summary>前フレームからのマウス水平移動量を返す。</summary>
	LONG GetMouseDeltaX() const {
		return mouse_.GetDeltaX();
	}

	/// <summary>前フレームからのマウス垂直移動量を返す。</summary>
	LONG GetMouseDeltaY() const {
		return mouse_.GetDeltaY();
	}

	/// <summary>このフレームのホイール回転量を返す。</summary>
	LONG GetMouseWheelDelta() const {
		return mouse_.GetWheelDelta();
	}

	/// <summary>クライアント領域上のマウス座標を返す。</summary>
	POINT GetMousePosition() const {
		return mouse_.GetPosition();
	}

	/// <summary>指定したマウスボタンが押されているかを返す。</summary>
	bool PushMouseButton(uint32_t button) const {
		return mouse_.PushButton(button);
	}

	/// <summary>指定したマウスボタンがこのフレームで押されたかを返す。</summary>
	bool TriggerMouseButton(uint32_t button) const {
		return mouse_.TriggerButton(button);
	}

	/// <summary>指定したマウスボタンがこのフレームで離されたかを返す。</summary>
	bool ReleaseMouseButton(uint32_t button) const {
		return mouse_.ReleaseButton(button);
	}

  public: // ===== ゲームパッド入力の従来互換API =====
	/// <summary>ゲームパッドが接続されているかを返す。</summary>
	bool IsPadConnected() const {
		return gamepad_.IsConnected();
	}

	/// <summary>指定したパッドボタンが押されているかを返す。</summary>
	bool PushPadButton(WORD buttons) const {
		return gamepad_.PushButton(buttons);
	}

	/// <summary>指定したパッドボタンがこのフレームで押されたかを返す。</summary>
	bool TriggerPadButton(WORD buttons) const {
		return gamepad_.TriggerButton(buttons);
	}

	/// <summary>指定したパッドボタンがこのフレームで離されたかを返す。</summary>
	bool ReleasePadButton(WORD buttons) const {
		return gamepad_.ReleaseButton(buttons);
	}

	/// <summary>左スティックの入力ベクトルを返す。</summary>
	Vector2 GetLeftStick() const {
		return gamepad_.GetLeftStick();
	}

	/// <summary>右スティックの入力ベクトルを返す。</summary>
	Vector2 GetRightStick() const {
		return gamepad_.GetRightStick();
	}

	/// <summary>左トリガーの押し込み量を返す。</summary>
	float GetLeftTrigger() const {
		return gamepad_.GetLeftTrigger();
	}

	/// <summary>右トリガーの押し込み量を返す。</summary>
	float GetRightTrigger() const {
		return gamepad_.GetRightTrigger();
	}

  public: // ===== デバイス非依存のアクションAPI =====
	/// <summary>名前付きのボタンアクションを追加する。</summary>
	InputActionId AddButtonAction(const std::string &name);

	/// <summary>名前付きの二次元軸アクションを追加する。</summary>
	InputActionId AddAxis2DAction(const std::string &name);

	/// <summary>ボタンアクションへキーボードキーを割り当てる。</summary>
	void BindKey(InputActionId action, BYTE key);

	/// <summary>ボタンアクションへマウスボタンを割り当てる。</summary>
	void BindMouseButton(InputActionId action, uint32_t button);

	/// <summary>ボタンアクションへゲームパッドボタンを割り当てる。</summary>
	void BindPadButton(InputActionId action, WORD button);

	/// <summary>二次元軸アクションへ上下左右のキーを割り当てる。</summary>
	void BindKeyboardAxis2D(InputActionId action, BYTE up, BYTE down, BYTE left, BYTE right);

	/// <summary>二次元軸アクションへ左スティックを割り当てる。</summary>
	void BindLeftStick(InputActionId action, float deadZone = 0.0f);

	/// <summary>指定したアクションが押されているかを返す。</summary>
	bool PushAction(InputActionId action) const;

	/// <summary>指定したアクションがこのフレームで押されたかを返す。</summary>
	bool TriggerAction(InputActionId action) const;

	/// <summary>指定したアクションがこのフレームで離されたかを返す。</summary>
	bool ReleaseAction(InputActionId action) const;

	/// <summary>指定した二次元軸アクションの入力値を返す。</summary>
	Vector2 GetActionAxis2D(InputActionId action) const;

	/// <summary>名前から入力アクション番号を検索する。</summary>
	InputActionId FindAction(const std::string &name) const;

  public: // ===== デバイス固有機能の取得 =====
	/// <summary>キーボード入力機能を返す。</summary>
	KeyboardInput &GetKeyboard() {
		return keyboard_;
	}

	/// <summary>読み取り専用のキーボード入力機能を返す。</summary>
	const KeyboardInput &GetKeyboard() const {
		return keyboard_;
	}

	/// <summary>マウス入力機能を返す。</summary>
	MouseInput &GetMouse() {
		return mouse_;
	}

	/// <summary>読み取り専用のマウス入力機能を返す。</summary>
	const MouseInput &GetMouse() const {
		return mouse_;
	}

	/// <summary>ゲームパッド入力機能を返す。</summary>
	GamepadInput &GetGamepad() {
		return gamepad_;
	}

	/// <summary>読み取り専用のゲームパッド入力機能を返す。</summary>
	const GamepadInput &GetGamepad() const {
		return gamepad_;
	}

	/// <summary>すべての入力デバイスが初期化済みかを返す。</summary>
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
		BindingDevice device = BindingDevice::Keyboard; // 入力元となるデバイス
		uint32_t code = 0;                              // デバイス固有のボタン番号
		bool operator==(const ButtonBinding &) const = default;
	};
	struct KeyboardAxis2DBinding {
		BYTE up = 0;    // 上方向として扱うキー
		BYTE down = 0;  // 下方向として扱うキー
		BYTE left = 0;  // 左方向として扱うキー
		BYTE right = 0; // 右方向として扱うキー
		bool operator==(const KeyboardAxis2DBinding &) const = default;
	};
	struct InputAction {
		std::string name;                                          // アクションを検索する名前
		InputActionType type = InputActionType::Button;            // アクションが扱う入力形式
		std::vector<ButtonBinding> buttonBindings;                 // ボタンとして評価する割り当て一覧
		std::vector<KeyboardAxis2DBinding> keyboardAxis2DBindings; // 二次元軸として評価するキー一覧
		bool useLeftStick = false;                                 // 左スティックを入力へ含めるか
		float stickDeadZone = 0.0f;                                // 左スティックへ適用する無効範囲
		bool previousPressed = false;                              // 前フレームの押下状態
		bool pressed = false;                                      // 現在フレームの押下状態
		Vector2 axis2D{};                                          // 現在フレームの二次元入力値
	};

	InputActionId AddAction(const std::string &name, InputActionType type);
	InputAction &RequireAction(InputActionId id, InputActionType expectedType);
	const InputAction &RequireAction(InputActionId id, InputActionType expectedType) const;
	void UpdateActions();
	bool IsBindingPressed(const ButtonBinding &binding) const;

	Microsoft::WRL::ComPtr<IDirectInput8> directInput_;        // DirectInputの管理インターフェース
	KeyboardInput keyboard_;                                   // キーボード状態の管理機能
	MouseInput mouse_;                                         // マウス状態の管理機能
	GamepadInput gamepad_;                                     // XInputゲームパッドの管理機能
	WinApp *winApp_ = nullptr;                                 // ウィンドウハンドルを提供する非所有参照
	std::vector<InputAction> actions_;                         // 番号順に保持する登録済みアクション
	std::unordered_map<std::string, InputActionId> actionIds_; // 名前からアクション番号を引く検索表
	bool initialized_ = false;                                 // 入力機能の初期化が完了しているか
};
