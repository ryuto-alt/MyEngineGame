#include "MatrixMath.h"
#include "Vector3Math.h"
#include <assert.h>
#include <cmath>

//行列の加法
Matrix4x4 MatrixMath::Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {
		m1.m[0][0] + m2.m[0][0],m1.m[0][1] + m2.m[0][1],m1.m[0][2] + m2.m[0][2],m1.m[0][3] + m2.m[0][3],
		m1.m[1][0] + m2.m[1][0],m1.m[1][1] + m2.m[1][1],m1.m[1][2] + m2.m[1][2],m1.m[1][3] + m2.m[1][3],
		m1.m[2][0] + m2.m[2][0],m1.m[2][1] + m2.m[2][1],m1.m[2][2] + m2.m[2][2],m1.m[2][3] + m2.m[2][3],
		m1.m[3][0] + m2.m[3][0],m1.m[3][1] + m2.m[3][1],m1.m[3][2] + m2.m[3][2],m1.m[3][3] + m2.m[3][3]
	};
	return result;
}

//行列の減法
Matrix4x4 MatrixMath::Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {
		m1.m[0][0] - m2.m[0][0],m1.m[0][1] - m2.m[0][1],m1.m[0][2] - m2.m[0][2],m1.m[0][3] - m2.m[0][3],
		m1.m[1][0] - m2.m[1][0],m1.m[1][1] - m2.m[1][1],m1.m[1][2] - m2.m[1][2],m1.m[1][3] - m2.m[1][3],
		m1.m[2][0] - m2.m[2][0],m1.m[2][1] - m2.m[2][1],m1.m[2][2] - m2.m[2][2],m1.m[2][3] - m2.m[2][3],
		m1.m[3][0] - m2.m[3][0],m1.m[3][1] - m2.m[3][1],m1.m[3][2] - m2.m[3][2],m1.m[3][3] - m2.m[3][3]
	};
	return result;
}

//行列の積
Matrix4x4 MatrixMath::Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int row = 0; row < 4; row++) {
		for (int colmn = 0; colmn < 4; colmn++) {
			result.m[row][colmn] = 0;
			for (int k = 0; k < 4; k++) {
				result.m[row][colmn] += m1.m[row][k] * m2.m[k][colmn];
			}
		}
	}
	return result;
}

//逆行列
Matrix4x4 MatrixMath::Inverse(const Matrix4x4& m) {
	float determinant =
		(m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3])
		+ (m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1])
		+ (m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2])
		- (m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1])
		- (m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3])
		- (m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2])
		- (m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3])
		- (m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1])
		- (m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2])
		+ (m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1])
		+ (m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3])
		+ (m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2])
		+ (m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3])
		+ (m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1])
		+ (m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2])
		- (m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1])
		- (m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3])
		- (m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2])
		- (m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0])
		- (m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0])
		- (m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0])
		+ (m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0])
		+ (m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0])
		+ (m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0]);

	Matrix4x4 result;
	result.m[0][0] = 1.0f / determinant *
		(m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2]
			- m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]);

	result.m[0][1] = 1.0f / determinant *
		(-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[2][1] * m.m[3][2]
			+ m.m[0][3] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]);

	result.m[0][2] = 1.0f / determinant *
		(m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2]
			- m.m[0][3] * m.m[1][2] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]);

	result.m[0][3] = 1.0f / determinant *
		(-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] - m.m[0][3] * m.m[1][1] * m.m[2][2]
			+ m.m[0][3] * m.m[1][2] * m.m[2][1] + m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]);

	result.m[1][0] = 1.0f / determinant *
		(-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[1][3] * m.m[2][0] * m.m[3][2]
			+ m.m[1][3] * m.m[2][2] * m.m[3][0] + m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]);

	result.m[1][1] = 1.0f / determinant *
		(m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2]
			- m.m[0][3] * m.m[2][2] * m.m[3][0] - m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]);

	result.m[1][2] = 1.0f / determinant *
		(-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] - m.m[0][3] * m.m[1][0] * m.m[3][2]
			+ m.m[0][3] * m.m[1][2] * m.m[3][0] + m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]);

	result.m[1][3] = 1.0f / determinant *
		(m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2]
			- m.m[0][3] * m.m[1][2] * m.m[2][0] - m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]);

	result.m[2][0] = 1.0f / determinant *
		(m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1]
			- m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]);

	result.m[2][1] = 1.0f / determinant *
		(-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[2][0] * m.m[3][1]
			+ m.m[0][3] * m.m[2][1] * m.m[3][0] + m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]);

	result.m[2][2] = 1.0f / determinant *
		(m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1]
			- m.m[0][3] * m.m[1][1] * m.m[3][0] - m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]);

	result.m[2][3] = 1.0f / determinant *
		(-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] - m.m[0][3] * m.m[1][0] * m.m[2][1]
			+ m.m[0][3] * m.m[1][1] * m.m[2][0] + m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]);

	result.m[3][0] = 1.0f / determinant *
		(-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] - m.m[1][2] * m.m[2][0] * m.m[3][1]
			+ m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]);

	result.m[3][1] = 1.0f / determinant *
		(m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1]
			- m.m[0][2] * m.m[2][1] * m.m[3][0] - m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]);

	result.m[3][2] = 1.0f / determinant *
		(-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] - m.m[0][2] * m.m[1][0] * m.m[3][1]
			+ m.m[0][2] * m.m[1][1] * m.m[3][0] + m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]);

	result.m[3][3] = 1.0f / determinant *
		(m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1]
			- m.m[0][2] * m.m[1][1] * m.m[2][0] - m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]);

	return result;
}

//転置行列
Matrix4x4 MatrixMath::Transpose(const Matrix4x4& m) {
	Matrix4x4 result;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

//単位行列の作成
Matrix4x4 MatrixMath::MakeIdentity4x4() {
	Matrix4x4 result = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
	return result;
}

//平行移動行列
Matrix4x4 MatrixMath::MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result = {
		1.0f,0,0,0,
		0,1.0f,0,0,
		0,0,1.0f,0,
		translate.x,translate.y,translate.z,1.0f
	};
	return result;
}

//拡大縮小行列
Matrix4x4 MatrixMath::MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 reslut = {
		scale.x,0,0,0,
		0,scale.y,0,0,
		0,0,scale.z,0,
		0,0,0,1
	};
	return reslut;
}

//X軸回転行列
Matrix4x4 MatrixMath::MakeRotateXMatrix(float radian) {
	Matrix4x4 reslut = {
		1.0f,0,0,0,
		0,std::cos(radian),std::sin(radian),0,
		0,std::sin(-radian),std::cos(radian),0,
		0,0,0,1.0f
	};
	return reslut;
}

//Y軸回転行列
Matrix4x4 MatrixMath::MakeRotateYMatrix(float radian) {
	Matrix4x4 reslut = {
		std::cos(radian),0,std::sin(-radian),0,
		0,1.0f,0,0,
		std::sin(radian),0,std::cos(radian),0,
		0,0,0,1.0f
	};
	return reslut;
}

//Z軸回転行列
Matrix4x4 MatrixMath::MakeRotateZMatrix(float radian) {
	Matrix4x4 reslut = {
		std::cos(radian),std::sin(radian),0,0,
		std::sin(-radian),std::cos(radian),0,0,
		0,0,1.0f,0,
		0,0,0,1.0f
	};
	return reslut;
}

Matrix4x4 MatrixMath::MakeRotateMatrix(const Vector3& rotate) {
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateMatrix = rotateXMatrix * rotateYMatrix * rotateZMatrix;
	return rotateMatrix;
}

Matrix4x4 MatrixMath::MakeRotateAxisAngle(const Vector3& axis, float angle) {
	float cosA = std::cos(angle);
	float sinA = std::sin(angle);
	float oneMinusCosA = 1.0f - cosA;

	Matrix4x4 result;
	result.m[0][0] = axis.x * axis.x * oneMinusCosA + cosA;
	result.m[0][1] = axis.x * axis.y * oneMinusCosA + axis.z * sinA;
	result.m[0][2] = axis.x * axis.z * oneMinusCosA - axis.y * sinA;
	result.m[0][3] = 0.0f;

	result.m[1][0] = axis.y * axis.x * oneMinusCosA - axis.z * sinA;
	result.m[1][1] = axis.y * axis.y * oneMinusCosA + cosA;
	result.m[1][2] = axis.y * axis.z * oneMinusCosA + axis.x * sinA;
	result.m[1][3] = 0.0f;

	result.m[2][0] = axis.z * axis.x * oneMinusCosA + axis.y * sinA;
	result.m[2][1] = axis.z * axis.y * oneMinusCosA - axis.x * sinA;
	result.m[2][2] = axis.z * axis.z * oneMinusCosA + cosA;
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 MatrixMath::MakeRotateMatrix(const Quaternion& quaternion) {
	Matrix4x4 result;
	result.m[0][0] = 1.0f - 2.0f * quaternion.y * quaternion.y - 2.0f * quaternion.z * quaternion.z;
	result.m[0][1] = 2.0f * quaternion.x * quaternion.y + 2.0f * quaternion.w * quaternion.z;
	result.m[0][2] = 2.0f * quaternion.x * quaternion.z - 2.0f * quaternion.w * quaternion.y;
	result.m[0][3] = 0.0f;

	result.m[1][0] = 2.0f * quaternion.x * quaternion.y - 2.0f * quaternion.w * quaternion.z;
	result.m[1][1] = 1.0f - 2.0f * quaternion.x * quaternion.x - 2.0f * quaternion.z * quaternion.z;
	result.m[1][2] = 2.0f * quaternion.y * quaternion.z + 2.0f * quaternion.w * quaternion.x;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 2.0f * quaternion.x * quaternion.z + 2.0f * quaternion.w * quaternion.y;
	result.m[2][1] = 2.0f * quaternion.y * quaternion.z - 2.0f * quaternion.w * quaternion.x;
	result.m[2][2] = 1.0f - 2.0f * quaternion.x * quaternion.x - 2.0f * quaternion.y * quaternion.y;
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

//3次元アフィン変換行列
Matrix4x4 MatrixMath::MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 rotateMatrix = MakeRotateMatrix(rotate);
	Matrix4x4 result = {
		scale.x * rotateMatrix.m[0][0],scale.x * rotateMatrix.m[0][1],scale.x * rotateMatrix.m[0][2],0,
		scale.y * rotateMatrix.m[1][0],scale.y * rotateMatrix.m[1][1],scale.y * rotateMatrix.m[1][2],0,
		scale.z * rotateMatrix.m[2][0],scale.z * rotateMatrix.m[2][1],scale.z * rotateMatrix.m[2][2],0,
		translate.x,translate.y,translate.z,1.0f
	};
	return result;
}

Matrix4x4 MatrixMath::MakeAffineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& translate) {
	Matrix4x4 rotateMatrix = MakeRotateMatrix(rotate);
	Matrix4x4 result = {
		scale.x * rotateMatrix.m[0][0],scale.x * rotateMatrix.m[0][1],scale.x * rotateMatrix.m[0][2],0,
		scale.y * rotateMatrix.m[1][0],scale.y * rotateMatrix.m[1][1],scale.y * rotateMatrix.m[1][2],0,
		scale.z * rotateMatrix.m[2][0],scale.z * rotateMatrix.m[2][1],scale.z * rotateMatrix.m[2][2],0,
		translate.x,translate.y,translate.z,1.0f
	};
	return result;
}

//透視投影行列
Matrix4x4 MatrixMath::MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result = {
		1.0f / aspectRatio * (1.0f / std::tan(fovY / 2.0f)),0,0,0,
		0,1.0f / std::tan(fovY / 2.0f),0,0,
		0,0,farClip / (farClip - nearClip),1.0f,
		0,0,-(nearClip * farClip) / (farClip - nearClip),0
	};
	return result;
}

//正射影行列
Matrix4x4 MatrixMath::MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = {
		2.0f / (right - left),0,0,0,
		0,2.0f / (top - bottom),0,0,
		0,0,1.0f / (farClip - nearClip),0,
		(left + right) / (left - right),(top + bottom) / (bottom - top),
		nearClip / (nearClip - farClip),1.0f
	};
	return result;
}

//ビューポート変換行列
Matrix4x4 MatrixMath::MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result = {
		width / 2.0f,0,0,0,
		0,-height / 2.0f,0,0,
		0,0,maxDepth - minDepth,0,
		left + width / 2.0f, top + height / 2.0f,minDepth,1.0f
	};
	return result;
}

//Matrix4x4からVector3に座標変換
Vector3 MatrixMath::Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}

Vector3 MatrixMath::TransformNormal(const Vector3& v, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = v.x * matrix.m[0][0] + v.y * matrix.m[1][0] + v.z * matrix.m[2][0];
	result.y = v.x * matrix.m[0][1] + v.y * matrix.m[1][1] + v.z * matrix.m[2][1];
	result.z = v.x * matrix.m[0][2] + v.y * matrix.m[1][2] + v.z * matrix.m[2][2];

	return result;
}

Matrix4x4 MatrixMath::InverseTranspose(const Matrix4x4& m) {

	Matrix4x4 result = Inverse(m);
	result = Transpose(result);
	return result;
}

Matrix4x4 MatrixMath::DirectionToDirection(const Vector3& from, const Vector3& to) {

	Vector3 crossProduct = Vector3Math::Cross(from, to);
	float cosA = Vector3Math::Dot(from, to);
	float sinA = Vector3Math::Length(crossProduct);
	float oneMinusCosA = 1.0f - cosA;

	Vector3 axis;
	if (sinA < 1e-6) {
		// from と to が反対方向を向いている場合
		if (fabs(from.x) > fabs(from.y)) {
			axis = Vector3Math::Normalize(Vector3{ from.y,-from.x,0.0f });
		} else {
			axis = Vector3Math::Normalize(Vector3{ from.z, 0.0f, -from.x });
		}
	} else {
		axis = Vector3Math::Normalize(crossProduct);
	}

	Matrix4x4 result;
	result.m[0][0] = axis.x * axis.x * oneMinusCosA + cosA;
	result.m[0][1] = axis.x * axis.y * oneMinusCosA + axis.z * sinA;
	result.m[0][2] = axis.x * axis.z * oneMinusCosA - axis.y * sinA;
	result.m[0][3] = 0.0f;

	result.m[1][0] = axis.y * axis.x * oneMinusCosA - axis.z * sinA;
	result.m[1][1] = axis.y * axis.y * oneMinusCosA + cosA;
	result.m[1][2] = axis.y * axis.z * oneMinusCosA + axis.x * sinA;
	result.m[1][3] = 0.0f;

	result.m[2][0] = axis.z * axis.x * oneMinusCosA + axis.y * sinA;
	result.m[2][1] = axis.z * axis.y * oneMinusCosA - axis.x * sinA;
	result.m[2][2] = axis.z * axis.z * oneMinusCosA + cosA;
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;
	return result;
}

//void MatrixMath::MatirxScreenPrintf(int x, int y, const Matrix4x4& matrix,const char* label) {
//	Novice::ScreenPrintf(x, y, "%s", label);
//	for (int row = 0; row < 4; row++){
//		for (int column = 0; column < 4; column++){	
//			Novice::ScreenPrintf(x + column * 60, y + 20 + row * 20, "%6.02f", matrix.m[row][column]);
//		}
//	}
//}
