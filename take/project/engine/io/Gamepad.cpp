#include "Gamepad.h"
// c++
#include <algorithm>
#include <cassert>

///-------------------------------------------///
/// デストラクタ
///-------------------------------------------///
GamePad::~GamePad() {}

///-------------------------------------------///
/// 初期化　対応済み
///-------------------------------------------///
void GamePad::Initialize() {
	ZeroMemory(currentState_, sizeof(currentState_));
	ZeroMemory(previousState_, sizeof(previousState_));
	ZeroMemory(currentDIState_, sizeof(currentDIState_));
	ZeroMemory(previousDIState_, sizeof(previousDIState_));

	// XInput / DirectInput のボタンマッピング
	buttonMapping_ = {
	    {GamepadButtonType::A,          {static_cast<WORD>(XINPUT_GAMEPAD_A), 0}             }, // Aボタン
	    {GamepadButtonType::B,          {static_cast<WORD>(XINPUT_GAMEPAD_B), 1}             }, // Bボタン
	    {GamepadButtonType::X,          {static_cast<WORD>(XINPUT_GAMEPAD_X), 2}             }, // Xボタン
	    {GamepadButtonType::Y,          {static_cast<WORD>(XINPUT_GAMEPAD_Y), 3}             }, // Yボタン
	    {GamepadButtonType::RB,         {static_cast<WORD>(XINPUT_GAMEPAD_RIGHT_SHOULDER), 5}}, // RB
	    {GamepadButtonType::LB,         {static_cast<WORD>(XINPUT_GAMEPAD_LEFT_SHOULDER), 4} }, // LB
	    {GamepadButtonType::DPadUP,     {static_cast<WORD>(XINPUT_GAMEPAD_DPAD_UP), 10}      }, // DPad ↑
	    {GamepadButtonType::DPadDOWN,   {static_cast<WORD>(XINPUT_GAMEPAD_DPAD_DOWN), 12}    }, // DPad ↓
	    {GamepadButtonType::DPadLEFT,   {static_cast<WORD>(XINPUT_GAMEPAD_DPAD_LEFT), 13}    }, // DPad ←
	    {GamepadButtonType::DPadRIGHT,  {static_cast<WORD>(XINPUT_GAMEPAD_DPAD_RIGHT), 11}   }, // DPad →
	    {GamepadButtonType::LeftStick,  {static_cast<WORD>(XINPUT_GAMEPAD_LEFT_THUMB), 6}    }, // Lスティック押し込み
	    {GamepadButtonType::RightStick, {static_cast<WORD>(XINPUT_GAMEPAD_RIGHT_THUMB), 7}   }, // Rスティック押し込み
	    {GamepadButtonType::Start,      {static_cast<WORD>(XINPUT_GAMEPAD_START), 14}        }, // Start
	};
}

///-------------------------------------------///
/// 更新 対応済み
///-------------------------------------------///
void GamePad::Update() {
	for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
		previousState_[i] = currentState_[i];
		ZeroMemory(&currentState_[i], sizeof(XINPUT_STATE));
		XInputGetState(i, &currentState_[i]);

		previousDIState_[i] = currentDIState_[i];
		ZeroMemory(&currentDIState_[i], sizeof(DIJOYSTATE2));
	}
}

///-------------------------------------------///
/// コントローラースティックの取得
///-------------------------------------------///
// XInput
bool GamePad::GetJoystickState(int stickNo, XINPUT_STATE& out) const {
	if (stickNo < 0 || stickNo >= XUSER_MAX_COUNT)
		return false;
	out = currentState_[stickNo];
	return true;
}
bool GamePad::GetJoystickStatePrevious(int stickNo, XINPUT_STATE& out) const {
	if (stickNo < 0 || stickNo >= XUSER_MAX_COUNT)
		return false;
	out = previousState_[stickNo];
	return true;
}
// DirectInput
bool GamePad::GetJoystickState(int stickNo, DIJOYSTATE2& out) const {
	if (stickNo < 0 || stickNo >= XUSER_MAX_COUNT)
		return false;
	out = currentDIState_[stickNo];
	return true;
}
bool GamePad::GetJoystickStatePrevious(int stickNo, DIJOYSTATE2& out) const {
	if (stickNo < 0 || stickNo >= XUSER_MAX_COUNT)
		return false;
	out = previousDIState_[stickNo];
	return true;
}

///-------------------------------------------///
/// 指定したボタンの XInput / DirectInput マッピングを取得
///-------------------------------------------///
std::pair<WORD, int> GamePad::ConvertToButton(GamepadButtonType button) const {
	auto it = buttonMapping_.find(button);
	if (it != buttonMapping_.end()) {
		return it->second;
	}
	return {static_cast<WORD>(0), -1}; // 無効なボタン
}

///-------------------------------------------///
/// ボタンを押している間　対応済み
///-------------------------------------------///
bool GamePad::PushButton(int stickNo, GamepadButtonType button) const {
	auto [xInputButton, dInputButton] = ConvertToButton(button);

	// XInput の通常ボタン
	if ((currentState_[stickNo].Gamepad.wButtons & xInputButton)) {
		return true;
	}

	// DirectInput のボタン処理
	if (dInputButton >= 0 && (currentDIState_[stickNo].rgbButtons[dInputButton] & 0x80)) {
		return true;
	}

	// LT（左トリガー） DirectInput 追加
	if (button == GamepadButtonType::LT) {
		return (currentState_[stickNo].Gamepad.bLeftTrigger > TRIGGER_THRESHOLD || currentDIState_[stickNo].lZ > 32767); // DirectInput のトリガー処理
	}

	// RT（右トリガー） DirectInput 追加
	if (button == GamepadButtonType::RT) {
		return (currentState_[stickNo].Gamepad.bRightTrigger > TRIGGER_THRESHOLD || currentDIState_[stickNo].lRz > 32767);
	}

	return false;
}

///-------------------------------------------///
/// ボタンを押した瞬間　対応済み
///-------------------------------------------///
bool GamePad::TriggerButton(int stickNo, GamepadButtonType button) const {
	auto [xInputButton, dInputButton] = ConvertToButton(button);

	// XInput の通常ボタン
	if ((currentState_[stickNo].Gamepad.wButtons & xInputButton) && !(previousState_[stickNo].Gamepad.wButtons & xInputButton)) {
		return true;
	}

	// DirectInput のボタン処理
	if (dInputButton >= 0 && (currentDIState_[stickNo].rgbButtons[dInputButton] & 0x80) && !(previousDIState_[stickNo].rgbButtons[dInputButton] & 0x80)) {
		return true;
	}

	// LT（左トリガー
	if (button == GamepadButtonType::LT) {
		return (currentState_[stickNo].Gamepad.bLeftTrigger > TRIGGER_THRESHOLD && previousState_[stickNo].Gamepad.bLeftTrigger <= TRIGGER_THRESHOLD) ||
		       (currentDIState_[stickNo].lZ > 32767 && previousDIState_[stickNo].lZ <= 32767);
	}

	// RT（右トリガー）
	if (button == GamepadButtonType::RT) {
		return (currentState_[stickNo].Gamepad.bRightTrigger > TRIGGER_THRESHOLD && previousState_[stickNo].Gamepad.bRightTrigger <= TRIGGER_THRESHOLD) ||
		       (currentDIState_[stickNo].lRz > 32767 && previousDIState_[stickNo].lRz <= 32767);
	}

	return false;
}

///-------------------------------------------///
/// ボタンを離した瞬間 対応済み
///-------------------------------------------///
bool GamePad::ReleaseButton(int stickNo, GamepadButtonType button) const {
	auto [xInputButton, dInputButton] = ConvertToButton(button);

	// XInput の通常ボタン
	if (!(currentState_[stickNo].Gamepad.wButtons & xInputButton) && (previousState_[stickNo].Gamepad.wButtons & xInputButton)) {
		return true;
	}

	// DirectInput のボタン処理
	if (dInputButton >= 0 && !(currentDIState_[stickNo].rgbButtons[dInputButton] & 0x80) && (previousDIState_[stickNo].rgbButtons[dInputButton] & 0x80)) {
		return true;
	}

	// LT（左トリガー） DirectInput 追加
	if (button == GamepadButtonType::LT) {
		return (currentState_[stickNo].Gamepad.bLeftTrigger <= TRIGGER_THRESHOLD && previousState_[stickNo].Gamepad.bLeftTrigger > TRIGGER_THRESHOLD) ||
		       (currentDIState_[stickNo].lZ <= 32767 && previousDIState_[stickNo].lZ > 32767);
	}

	// RT（右トリガー） DirectInput 追加
	if (button == GamepadButtonType::RT) {
		return (currentState_[stickNo].Gamepad.bRightTrigger <= TRIGGER_THRESHOLD && previousState_[stickNo].Gamepad.bRightTrigger > TRIGGER_THRESHOLD) ||
		       (currentDIState_[stickNo].lRz <= 32767 && previousDIState_[stickNo].lRz > 32767);
	}

	return false;
}

///-------------------------------------------///
/// スティックの状況を取得 対応済み
///-------------------------------------------///
// 左スティックの状況を取得
StickState GamePad::GetLeftStickState(int stickNo) const {
	StickState state = {0.0f, 0.0f};
	if (stickNo < 0 || stickNo >= XUSER_MAX_COUNT)
		return state;

	// XInput のスティック値（-1.0f ～ 1.0f）
	float lxXInput = static_cast<float>(currentState_[stickNo].Gamepad.sThumbLX) / NORMALIZE_RANGE;
	float lyXInput = static_cast<float>(currentState_[stickNo].Gamepad.sThumbLY) / NORMALIZE_RANGE;

	// DirectInput のスティック値（0 ～ 65535 → -1.0f ～ 1.0f に変換）
	float lxDInput = (static_cast<float>(currentDIState_[stickNo].lX) - 32767.0f) / 32767.0f;
	float lyDInput = (static_cast<float>(currentDIState_[stickNo].lY) - 32767.0f) / 32767.0f; // Y軸は反転

	// デッドゾーン処理（微小な入力を無視）
	if (std::abs(lxXInput) < DEADZONE)
		lxXInput = 0.0f;
	if (std::abs(lyXInput) < DEADZONE)
		lyXInput = 0.0f;
	if (std::abs(lxDInput) < DEADZONE)
		lxDInput = 0.0f;
	if (std::abs(lyDInput) < DEADZONE)
		lyDInput = 0.0f;

	// XInput と DirectInput の大きい方を採用
	state.x = std::max<float>(lxXInput, std::clamp<float>(lxDInput, -1.0f, 1.0f));
	state.y = std::max<float>(lyXInput, std::clamp<float>(lyDInput, -1.0f, 1.0f));

	return state;
}
// 右スティックの状況を取得
StickState GamePad::GetRightStickState(int stickNo) const {
	StickState state = {0.0f, 0.0f};
	if (stickNo < 0 || stickNo >= XUSER_MAX_COUNT)
		return state;

	// XInput のスティック値（-1.0f ～ 1.0f）
	float rxXInput = static_cast<float>(currentState_[stickNo].Gamepad.sThumbRX) / NORMALIZE_RANGE;
	float ryXInput = static_cast<float>(currentState_[stickNo].Gamepad.sThumbRY) / NORMALIZE_RANGE;

	// DirectInput のスティック値（0 ～ 65535 → -1.0f ～ 1.0f に変換）
	float rxDInput = (static_cast<float>(currentDIState_[stickNo].lRx) - 32767.0f) / 32767.0f;
	float ryDInput = (static_cast<float>(currentDIState_[stickNo].lRy) - 32767.0f) / 32767.0f; // Y軸は反転

	// デッドゾーン処理（微小な入力を無視）
	if (std::abs(rxXInput) < DEADZONE)
		rxXInput = 0.0f;
	if (std::abs(ryXInput) < DEADZONE)
		ryXInput = 0.0f;
	if (std::abs(rxDInput) < DEADZONE)
		rxDInput = 0.0f;
	if (std::abs(ryDInput) < DEADZONE)
		ryDInput = 0.0f;

	// XInput と DirectInput の大きい方を採用
	state.x = std::max<float>(rxXInput, std::clamp<float>(rxDInput, -1.0f, 1.0f));
	state.y = std::max<float>(ryXInput, std::clamp<float>(ryDInput, -1.0f, 1.0f));

	return state;
}

///-------------------------------------------///
/// 指定スティックの値を取得 対応済み
///-------------------------------------------///
float GamePad::GetStickValue(int stickNo, GamepadValueType valueType) const {
	if (stickNo < 0 || stickNo >= XUSER_MAX_COUNT)
		return 0.0f;

	// XInput / DirectInput のスティック値
	float xInputValue = 0.0f;
	float dInputValue = 0.0f;

	switch (valueType) {
	case GamepadValueType::LeftStickX:
		xInputValue = static_cast<float>(currentState_[stickNo].Gamepad.sThumbLX) / NORMALIZE_RANGE;
		dInputValue = static_cast<float>(currentDIState_[stickNo].lX) / 32767.0f; // DirectInput のスケール調整
		break;
	case GamepadValueType::LeftStickY:
		xInputValue = static_cast<float>(currentState_[stickNo].Gamepad.sThumbLY) / NORMALIZE_RANGE;
		dInputValue = static_cast<float>(currentDIState_[stickNo].lY) / 32767.0f;
		break;
	case GamepadValueType::RightStickX:
		xInputValue = static_cast<float>(currentState_[stickNo].Gamepad.sThumbRX) / NORMALIZE_RANGE;
		dInputValue = static_cast<float>(currentDIState_[stickNo].lRx) / 32767.0f;
		break;
	case GamepadValueType::RightStickY:
		xInputValue = static_cast<float>(currentState_[stickNo].Gamepad.sThumbRY) / NORMALIZE_RANGE;
		dInputValue = static_cast<float>(currentDIState_[stickNo].lRy) / 32767.0f;
		break;
	default:
		return 0.0f;
	}

	// デッドゾーン処理（微小な入力を無視）
	if (std::abs(xInputValue) < DEADZONE)
		xInputValue = 0.0f;
	if (std::abs(dInputValue) < DEADZONE)
		dInputValue = 0.0f;

	// XInput と DirectInput の値のうち、大きい方を採用
	return std::max<float>(xInputValue, std::clamp<float>(dInputValue, -1.0f, 1.0f));
}

///-------------------------------------------///
/// ボタンの押し込み量を取得　対応済み
///-------------------------------------------///
float GamePad::GetTriggerValue(int stickNo, GamepadButtonType button) const {
	if (stickNo < 0 || stickNo >= XUSER_MAX_COUNT)
		return 0.0f;

	switch (button) {
		// XInput / DirectInput の LT（左トリガー）
	case GamepadButtonType::LT: {
		float lTriggerDI = static_cast<float>(currentDIState_[stickNo].lZ) / 65535.0f;
		float lTriggerX = static_cast<float>(currentState_[stickNo].Gamepad.bLeftTrigger) / 255.0f;
		return std::max<float>(lTriggerX, std::clamp<float>(lTriggerDI, 0.0f, 1.0f));
	}

	// XInput / DirectInput の RT（右トリガー）
	case GamepadButtonType::RT: {
		float rTriggerDI = static_cast<float>(currentDIState_[stickNo].lRz) / 65535.0f;
		float rTriggerX = static_cast<float>(currentState_[stickNo].Gamepad.bRightTrigger) / 255.0f;
		return std::max<float>(rTriggerX, std::clamp<float>(rTriggerDI, 0.0f, 1.0f));
	}

	// XInput / DirectInput の LB（左バンパー）
	case GamepadButtonType::LB:
		return ((currentState_[stickNo].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) || (currentDIState_[stickNo].rgbButtons[4] & 0x80)) ? 1.0f : 0.0f;

		// XInput / DirectInput の RB（右バンパー）
	case GamepadButtonType::RB:
		return ((currentState_[stickNo].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) || (currentDIState_[stickNo].rgbButtons[5] & 0x80)) ? 1.0f : 0.0f;

	default:
		return 0.0f;
	}
}
