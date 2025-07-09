#pragma once
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include <cmath>
#include <numbers>

namespace Easing {

	//===============線形補間====================//

	float Lerp(float startPos, float endPos, float easedT);

	Vector3 Lerp(Vector3 startPos, Vector3 endPos, float easedT);

	//球面線形補間
	Quaternion Slerp(Quaternion q0, Quaternion q1, float t);

	float EaseOut(float x);

	float EaseIn(float x);

	float EaseInOut(float x);
};






