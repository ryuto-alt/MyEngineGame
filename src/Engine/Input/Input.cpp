#include "Input.h"
#include <cassert>
#include <cmath>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"xinput.lib")

// デッドゾーン設定
const float Input::XBOX_STICK_DEADZONE = 0.2f;

void Input::Initialize(WinApp* winApp)
{
	winApp_ = winApp;
	HRESULT hr;

	//DirectInputのインスタンスを生成
	hr = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));

	//キーボードデバイス生成
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(hr));

	//入力データ形式のセット
	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	//排他制御レベルのセット
	hr = keyboard->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	//マウスデバイス生成
	hr = directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
	assert(SUCCEEDED(hr));

	//入力データ形式のセット
	hr = mouse->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(hr));

	//排他制御レベルのセット
	hr = mouse->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(hr));
	
	// ウィンドウ中央座標を計算
	RECT windowRect;
	GetClientRect(winApp->GetHwnd(), &windowRect);
	windowCenter_.x = (windowRect.right - windowRect.left) / 2;
	windowCenter_.y = (windowRect.bottom - windowRect.top) / 2;
	
	// マウス状態の初期化
	memset(&mouseState_, 0, sizeof(mouseState_));
	memset(&previousMouseState_, 0, sizeof(previousMouseState_));
	
	// Xboxコントローラー状態の初期化
	for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
		memset(&xboxControllerState_[i], 0, sizeof(XINPUT_STATE));
		memset(&previousXboxControllerState_[i], 0, sizeof(XINPUT_STATE));
		xboxControllerConnected_[i] = false;
	}
}

void Input::Update()
{
	//前回のキー入力を保存
	memcpy(preKey, key, sizeof(key));
	//キーボード情報の取得
	keyboard->Acquire();
	//全キーボード入力情報を取得
	keyboard->GetDeviceState(sizeof(key), key);

	// 前回のマウス状態を保存
	memcpy(&previousMouseState_, &mouseState_, sizeof(mouseState_));
	//マウス情報の取得
	mouse->Acquire();
	mouse->GetDeviceState(sizeof(mouseState_), &mouseState_);
	
	// Xboxコントローラー状態の更新
	for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
		// 前回の状態を保存
		memcpy(&previousXboxControllerState_[i], &xboxControllerState_[i], sizeof(XINPUT_STATE));
		
		// 現在の状態を取得
		DWORD result = XInputGetState(i, &xboxControllerState_[i]);
		xboxControllerConnected_[i] = (result == ERROR_SUCCESS);
	}
}

void Input::Finalize()
{
	// マウスカーソルを必ず表示に戻す
	SetMouseCursor(true);

	// デバイスの解放
	if (mouse) {
		mouse->Unacquire();
		mouse->Release();
		mouse = nullptr;
	}

	if (keyboard) {
		keyboard->Unacquire();
		keyboard->Release();
		keyboard = nullptr;
	}

	if (directInput) {
		directInput->Release();
		directInput = nullptr;
	}
}

bool Input::PushKey(BYTE keyNumber)
{
	if (key[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	if (key[keyNumber] && !preKey[keyNumber]) {
		return true;
	}
	return false;
}

HRESULT Input::GetMouseState(DIMOUSESTATE* mouseState)
{
	return mouse->GetDeviceState(sizeof(DIMOUSESTATE), mouseState);
}

void Input::SetMouseCursor(bool visible)
{
	if (visible) {
		// マウスカーソルを表示
		while (ShowCursor(TRUE) < 0) {}
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	} else {
		// マウスカーソルを完全に非表示
		while (ShowCursor(FALSE) >= 0) {}
		SetCursor(NULL);
	}
}

void Input::GetMouseMovement(float& deltaX, float& deltaY)
{
	// DirectInputのマウス移動量を取得
	deltaX = static_cast<float>(mouseState_.lX);
	deltaY = static_cast<float>(mouseState_.lY);
}

void Input::ResetMouseCenter()
{
	// マウスカーソルをウィンドウ中央に移動
	POINT centerPoint = windowCenter_;
	ClientToScreen(winApp_->GetHwnd(), &centerPoint);
	SetCursorPos(centerPoint.x, centerPoint.y);
}

// Xboxコントローラー関連の実装
bool Input::IsXboxControllerConnected(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= XUSER_MAX_COUNT) return false;
	return xboxControllerConnected_[playerIndex];
}

bool Input::IsXboxButtonPressed(int button, int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= XUSER_MAX_COUNT) return false;
	if (!xboxControllerConnected_[playerIndex]) return false;
	
	return (xboxControllerState_[playerIndex].Gamepad.wButtons & button) != 0;
}

bool Input::IsXboxButtonTriggered(int button, int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= XUSER_MAX_COUNT) return false;
	if (!xboxControllerConnected_[playerIndex]) return false;
	
	bool currentPressed = (xboxControllerState_[playerIndex].Gamepad.wButtons & button) != 0;
	bool previousPressed = (previousXboxControllerState_[playerIndex].Gamepad.wButtons & button) != 0;
	
	return currentPressed && !previousPressed;
}

float Input::GetXboxLeftStickX(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= XUSER_MAX_COUNT) return 0.0f;
	if (!xboxControllerConnected_[playerIndex]) return 0.0f;
	
	float x = static_cast<float>(xboxControllerState_[playerIndex].Gamepad.sThumbLX) / 32767.0f;
	return (std::abs(x) < XBOX_STICK_DEADZONE) ? 0.0f : x;
}

float Input::GetXboxLeftStickY(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= XUSER_MAX_COUNT) return 0.0f;
	if (!xboxControllerConnected_[playerIndex]) return 0.0f;
	
	float y = static_cast<float>(xboxControllerState_[playerIndex].Gamepad.sThumbLY) / 32767.0f;
	return (std::abs(y) < XBOX_STICK_DEADZONE) ? 0.0f : y;
}

float Input::GetXboxRightStickX(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= XUSER_MAX_COUNT) return 0.0f;
	if (!xboxControllerConnected_[playerIndex]) return 0.0f;
	
	float x = static_cast<float>(xboxControllerState_[playerIndex].Gamepad.sThumbRX) / 32767.0f;
	return (std::abs(x) < XBOX_STICK_DEADZONE) ? 0.0f : x;
}

float Input::GetXboxRightStickY(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= XUSER_MAX_COUNT) return 0.0f;
	if (!xboxControllerConnected_[playerIndex]) return 0.0f;
	
	float y = static_cast<float>(xboxControllerState_[playerIndex].Gamepad.sThumbRY) / 32767.0f;
	return (std::abs(y) < XBOX_STICK_DEADZONE) ? 0.0f : y;
}

float Input::GetXboxLeftTrigger(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= XUSER_MAX_COUNT) return 0.0f;
	if (!xboxControllerConnected_[playerIndex]) return 0.0f;
	
	return static_cast<float>(xboxControllerState_[playerIndex].Gamepad.bLeftTrigger) / 255.0f;
}

float Input::GetXboxRightTrigger(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= XUSER_MAX_COUNT) return 0.0f;
	if (!xboxControllerConnected_[playerIndex]) return 0.0f;
	
	return static_cast<float>(xboxControllerState_[playerIndex].Gamepad.bRightTrigger) / 255.0f;
}