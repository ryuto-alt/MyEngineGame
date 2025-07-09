#pragma once
#define DIRECTINPUT_VERSION 0x0800 //DirectInputのバージョン指定
#include <dinput.h>
#include <Xinput.h>
#include <wrl.h>
#include <cstdint>
#include <vector>
#include <memory>

#include "Vector2.h"
#include "Gamepad.h"

class WinApp;
class Input {
public:
	struct MouseMove {
		LONG lX;
		LONG lY;
		LONG lZ;
	};

	//enum class PadType {
	//	DirectInput,
	//	XInput,
	//};


	//union State {
	//	XINPUT_STATE xInput_;         // XInput の生の入力データ
	//	DIJOYSTATE2 directInput_;     // DirectInput の生の入力データ
	//	Vector2 processedLeftStick_;  // デッドゾーン処理後の左スティック
	//	Vector2 processedRightStick_; // デッドゾーン処理後の右スティック
	//};

	//struct Joystick {
	//	Microsoft::WRL::ComPtr<IDirectInputDevice8> device_;
	//	int32_t deadZoneL_;
	//	int32_t deadZoneR_;
	//	PadType type_;
	//	State state_;
	//	State statePre_;
	//};

public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static Input* GetInstance();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="winApp"></param>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="winApp"></param>
	void Update();

	void Finalize();

	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>押されているか</returns>
	bool PushKey(BYTE keyNumber);

	bool TriggerKey(BYTE keyNumber);

	

	/// <summary>
	/// 全マウス情報取得
	/// </summary>
	/// <returns>マウス情報</returns>
	const DIMOUSESTATE2& GetAllMouse() const;

	/// <summary>
	/// マウスの押下をチェック
	/// </summary>
	/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
	/// <returns>押されているか</returns>
	bool IsPressMouse(int32_t mouseNumber) const;

	/// <summary>
	/// マウスのトリガーをチェック。押した瞬間だけtrueになる
	/// </summary>
	/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
	/// <returns>トリガーか</returns>
	bool IsTriggerMouse(int32_t buttonNumber) const;

	/// <summary>
	/// マウス移動量を取得
	/// </summary>
	/// <returns>マウス移動量</returns>
	MouseMove GetMouseMove();

	/// <summary>
	/// ホイールスクロール量を取得する
	/// </summary>
	/// <returns>ホイールスクロール量。奥側に回したら+。Windowsの設定で逆にしてたら逆</returns>
	int32_t GetWheel() const;

	/// <summary>
	/// マウスの位置を取得する（ウィンドウ座標系）
	/// </summary>
	/// <returns>マウスの位置</returns>
	const Vector2& GetCursorPosition() const;

	//====================================================================
	//		GamePad
	//====================================================================

	//ゲームパッドの状態を取得(XInput)
	//bool GetJoystickState(int stickNo, XINPUT_STATE& out) const;

	////ゲームパッドの状態を取得(DirectInput)
	//bool GetJoystickState(int stickNo, DIJOYSTATE2& out) const;

	////前回のゲームパッドの状態を取得(XInput)
	//bool GetJoystickStatePrevious(int stickNo, XINPUT_STATE& out) const;

	////前回のゲームパッドの状態を取得(DirectInput)
	//bool GetJoystickStatePrevious(int stickNo, DIJOYSTATE2& out) const;

	//ゲームパッドの押下チェック
	bool PushButton(int stickNo, GamepadButtonType button) const;

	bool TriggerButton(int stickNo, GamepadButtonType button) const;

	bool ReleaseButton(int stickNo, GamepadButtonType button) const;

	//ボタンの押し込み量を取得
	float GetTriggerValue(int stickNo, GamepadButtonType button) const;

	//スティックの状況を取得
	StickState GetLeftStickState(int stickNo) const;

	StickState GetRightStickState(int stickNo) const;

	float GetStickValue(int stickNo, GamepadValueType valueType) const;

private:

	Input() = default;
	~Input() = default;
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

private:

	//シングルトンインスタンス
	static Input* instance_;

	ComPtr<IDirectInput8> directInput_ = nullptr;
	ComPtr<IDirectInputDevice8> keyboardDevice_ = nullptr;
	ComPtr<IDirectInputDevice8> mouseDevice_ = nullptr;

	std::unique_ptr<GamePad> gamePad_;
	
	//WindowsAPI
	WinApp* winApp_ = nullptr;
	//ウィンドウハンドル
	//HWND hwnd_;
	//キー情報
	BYTE key[256] = {};
	//前回フレームのキーデータ
	BYTE preKey[256] = {};
	DIMOUSESTATE2 mouse_;
	DIMOUSESTATE2 mousePre_;
	Vector2 mousePosition_;
};

