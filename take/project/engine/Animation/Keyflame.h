#pragma once
#include "Vector3.h"
#include "Quaternion.h"

template <typename T>
struct Keyflame {

	float time;
	T value;
};

using KeyflameVector3 = Keyflame<Vector3>;
using KeyflameQuaternion = Keyflame<Quaternion>;