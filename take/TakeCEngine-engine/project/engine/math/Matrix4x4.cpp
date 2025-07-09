#include "Matrix4x4.h"
#include "MatrixMath.h"

Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2) {
	return MatrixMath::Add(m1, m2);
}

Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2) {
	return MatrixMath::Subtract(m1, m2);
}

Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) {
	return MatrixMath::Multiply(m1, m2);
}

Matrix4x4 operator*(const Matrix4x4& m, float scalar) {
	Matrix4x4 result;
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			result.m[row][column] = m.m[row][column] * scalar;
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::operator+=(const Matrix4x4& matrix) {
	return MatrixMath::Add(*this, matrix);
}

Matrix4x4 Matrix4x4::operator-=(const Matrix4x4& matrix) {
	return MatrixMath::Subtract(*this, matrix);
}

Matrix4x4 Matrix4x4::operator*=(const Matrix4x4& matrix) {
	
	Matrix4x4 result;
	for (int row = 0; row < 4; row++) {
		for (int colmn = 0; colmn < 4; colmn++) {
			m[row][colmn] = 0;
			for (int k = 0; k < 4; k++) {
				result.m[row][colmn] += m[row][k] * matrix.m[k][colmn];
			}
		}
	}
	*this = result;

	return *this;
}
