#pragma once
#include "Vector3.h"
#include "Vector4.h"

template <typename T>
struct Keyflame {

	float time;
	T value;
};

using KeyflameVector3 = Keyflame<Vector3>;
using KeyflameQuaternion = Keyflame<Vector4>;