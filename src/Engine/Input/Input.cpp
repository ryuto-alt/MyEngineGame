#include "Input.h"
#include <cassert>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

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