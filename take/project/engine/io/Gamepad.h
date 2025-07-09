#pragma once
#include "InputData.h"

///=====================================================/// 
///	コントローラー
///=====================================================///
class GamePad {
public:



	GamePad() =default;
	~GamePad();

	// 初期化
	void Initialize();
	// 更新
	void Update();

    // コントローラースティックの取得
    bool GetJoystickState(int stickNo, XINPUT_STATE& out) const;
    bool GetJoystickStatePrevious(int stickNo, XINPUT_STATE& out) const;
	bool GetJoystickState(int stickNo, DIJOYSTATE2& out) const;
	bool GetJoystickStatePrevious(int stickNo, DIJOYSTATE2& out) const;

    // コントローラーの押下チェック
    bool PushButton(int stickNo, GamepadButtonType button) const;
    bool TriggerButton(int stickNo, GamepadButtonType button) const;
    bool ReleaseButton(int stickNo, GamepadButtonType button) const;

    // ボタンの押し込み量を取得
    float GetTriggerValue(int stickNo, GamepadButtonType button) const;

    // スティックの状況を取得
    StickState GetLeftStickState(int stickNo) const;
    StickState GetRightStickState(int stickNo) const;
    float GetStickValue(int stickNo, GamepadValueType valueType) const;

private:

    static constexpr float NORMALIZE_RANGE = 32768.0f; // XInputのスティック最大値
    static constexpr float TRIGGER_THRESHOLD = 128.0f; // トリガーボタン判定のしきい値
    static constexpr float DEADZONE = 0.15f; // スティックのデッドゾーン（15%）

	// XInput 対応用
    XINPUT_STATE currentState_[XUSER_MAX_COUNT] = {};
    XINPUT_STATE previousState_[XUSER_MAX_COUNT] = {};

    // DirectInput 対応用
    DIJOYSTATE2 currentDIState_[XUSER_MAX_COUNT] = {};
    DIJOYSTATE2 previousDIState_[XUSER_MAX_COUNT] = {};

    std::map<GamepadButtonType, std::pair<WORD, int>> buttonMapping_;

private:
    // 指定したボタンの XInput / DirectInput マッピングを取得
    std::pair<WORD, int> ConvertToButton(GamepadButtonType button) const;
};

