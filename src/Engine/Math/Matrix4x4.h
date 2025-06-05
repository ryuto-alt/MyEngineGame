#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cmath>

struct Matrix4x4 {
	float m[4][4];
	
	static Matrix4x4 MakeIdentity() {
		Matrix4x4 result = {};
		result.m[0][0] = 1.0f;
		result.m[1][1] = 1.0f;
		result.m[2][2] = 1.0f;
		result.m[3][3] = 1.0f;
		return result;
	}
	
	static Matrix4x4 MakeAffineTransform(const Vector3& scale, const Vector4& rotation, const Vector3& translate) {
		Matrix4x4 result = MakeIdentity();
		
		float qx = rotation.x, qy = rotation.y, qz = rotation.z, qw = rotation.w;
		
		result.m[0][0] = scale.x * (1 - 2*qy*qy - 2*qz*qz);
		result.m[0][1] = scale.x * (2*qx*qy + 2*qw*qz);
		result.m[0][2] = scale.x * (2*qx*qz - 2*qw*qy);
		
		result.m[1][0] = scale.y * (2*qx*qy - 2*qw*qz);
		result.m[1][1] = scale.y * (1 - 2*qx*qx - 2*qz*qz);
		result.m[1][2] = scale.y * (2*qy*qz + 2*qw*qx);
		
		result.m[2][0] = scale.z * (2*qx*qz + 2*qw*qy);
		result.m[2][1] = scale.z * (2*qy*qz - 2*qw*qx);
		result.m[2][2] = scale.z * (1 - 2*qx*qx - 2*qy*qy);
		
		result.m[3][0] = translate.x;
		result.m[3][1] = translate.y;
		result.m[3][2] = translate.z;
		
		return result;
	}
	
	Matrix4x4 operator*(const Matrix4x4& other) const {
		Matrix4x4 result = {};
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					result.m[i][j] += m[i][k] * other.m[k][j];
				}
			}
		}
		return result;
	}
};