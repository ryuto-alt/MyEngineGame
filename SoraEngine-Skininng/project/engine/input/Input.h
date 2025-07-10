#pragma once
#include <Windows.h>

#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>
#include "WinApp.h"
#include <Vector2.h>
#include <array>
template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

class Input
{
	static Input* instance;
	Input() = default;
	~Input() = default;
	Input(Input&) = default;
	Input& operator=(Input&) = delete;

public: // インナークラス
	struct MouseMove {
		LONG lX;
		LONG lY;
		LONG lZ;
	};
public:
	//シングルトンインスタンスの取得
	static Input* GetInstance();
	//終了
	void Finalize();



	//初期化
	void Initialize(WinApp* winApp);
	//更新
	void Update();

	//キーの状態
	bool PushKey(BYTE keyNumber);//押してるとき
	bool TriggerKey(BYTE keyNumber);//押したとき


	//マウスの状態
	bool PushMouse(int buttonNumber);
	bool TriggerMouse(int buttonNumber);
	//マウスの座標
	const Vector2& GetMousePos()const { return mousePos; };
	//マウスの移動量
	MouseMove GetMouseMove()const {
		MouseMove move;
		move.lX = mouse.lX;
		move.lY = mouse.lY;
		move.lZ = mouse.lZ;
		return move;
	};
	////マウスのホイールの移動量
	//int32_t GetMouseWheel()const {

	//	return mouse.lZ;
	//};

	//ボタンの入力状態
	bool PushGamePadButton(WORD button);
	bool TriggerGamePadButton(WORD button);
	//スティックとトリガーの値取得
	float GetGamePadStickX(bool righ=false);
	float GetGamePadStickY(bool righ = false);
	BYTE GetGamePadTrigger(bool righ = false);

	// バイブレーション制御
	void SetVibration(float leftMotor, float rightMotor);



private:
	ComPtr<IDirectInput8>directInput = nullptr;
	BYTE key[256] = {};
	BYTE preKey[256] = {};
	ComPtr<IDirectInputDevice8>keyboard;
	WinApp* winApp_ = nullptr;

	Microsoft::WRL::ComPtr<IDirectInputDevice8> devMouse_;
	DIMOUSESTATE2 mouse;
	DIMOUSESTATE2 preMouse;
	Vector2 mousePos;

	//ゲームパッド
	XINPUT_STATE state_; // 現在のゲームパッド状態
	XINPUT_STATE prevState_; // 前回のゲームパッド状態
	bool gamepadConnected_ = false;


};

