#pragma once
#define DIRECTINPUT_VERSION 0x0800 //DirectInputのバージョン指定
#include <cstdint>
#include <map>
#include <cmath>
#include <dinput.h>
#include <Xinput.h>
#include <Windows.h>
#include <wrl.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

enum class GamepadButtonType {
	A,          // Aボタン
	B,          // Bボタン
	X,          // Xボタン
	Y,          // Yボタン
	LB,         // LBボタン
	RB,         // RBボタン
	LT,         // LTボタン
	RT,         // RTボタン
	LeftStick,  // 左スティック押し込み
	RightStick, // 右スティック押し込み
	DPadUP,		// 十字キー上
	DPadDOWN,	// 十字キー下
	DPadLEFT,	// 十字キー左
	DPadRIGHT,	// 十字キー右
	Start,
};

enum class GamepadValueType {
	LeftStickX,  // 左スティック X 軸
	LeftStickY,  // 左スティック Y 軸
	RightStickX, // 右スティック X 軸
	RightStickY, // 右スティック Y 軸
};

struct StickState {
	float x;
	float y;
};