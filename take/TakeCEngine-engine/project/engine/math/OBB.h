#pragma once
#include "Vector3.h"
struct OBB {
	Vector3 center;
	Vector3 axis[3];
	Vector3 halfSize;
};