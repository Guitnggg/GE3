#include "Input.h"

#include <cassert>
#include <stdexcept>

// DirectInputとGUIDを使用するためのライブラリ
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

void Input::Initialize(WinApp* winApp)
{
	// 受け取ったWinAppのインスタンスを記録する
	this->winApp = winApp;

	HRESULT result;

	// DirectInputのインスタンスを生成する
	result = DirectInput8Create(
		winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	if (FAILED(result)) { throw std::runtime_error("Failed to initialize DirectInput."); }

	// キーボードデバイスを生成する
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	if (FAILED(result)) { throw std::runtime_error("Failed to create the keyboard input device."); }

	// 入力データ形式をキーボード用に設定する
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result)) { throw std::runtime_error("Failed to set the keyboard data format."); }

	// アプリが前面の間だけ非排他で入力を受け取る
	result = keyboard->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	if (FAILED(result)) { throw std::runtime_error("Failed to set the keyboard cooperative level."); }
}

void Input::Update()
{
	// 前回のキー入力状態を保存する
	memcpy(keyPre, key, sizeof(key));

	// キーボード入力の取得を開始する
	HRESULT result = keyboard->Acquire();
	if (FAILED(result) && result != DIERR_OTHERAPPHASPRIO) {
		ZeroMemory(key, sizeof(key));
		return;
	}

	// 全キーの入力状態を取得する
	result = keyboard->GetDeviceState(sizeof(key), key);
	if (FAILED(result)) {
		ZeroMemory(key, sizeof(key));
	}
}

bool Input::PushKey(BYTE keyNumber)
{
	// 指定キーが押されていればtrueを返す
	if (key[keyNumber]) {
		return true;
	}

	// 押されていなければfalseを返す
	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	// 指定キーが反応していればtrueを返す
	return (key[keyNumber] & 0x80) != 0 && (keyPre[keyNumber] & 0x80) == 0;
}
