#pragma once
#include "Matrix4x4.h"

struct TransformMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 WorldInverseTranspose;
};