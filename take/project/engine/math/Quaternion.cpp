#include "Quaternion.h"
#include "Vector3Math.h"
#include "MatrixMath.h"
#include <cmath>
#include <numbers>

Quaternion QuaternionMath::Multiply(const Quaternion& q1, const Quaternion& q2) {
	Quaternion result;
	result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
	result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
	result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
	result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
	return result;
}

Quaternion QuaternionMath::Multiply(const Quaternion& q, float s) {
	Quaternion result;
	result.x = q.x * s;
	result.y = q.y * s;
	result.z = q.z * s;
	result.w = q.w * s;
	return result;
}

float QuaternionMath::Dot(const Quaternion& lhs, const Quaternion& rhs) {
	float result = lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
	return result;
}

Quaternion QuaternionMath::IdentityQuaternion() {
	Quaternion result;
	result.x = 0.0f;
	result.y = 0.0f;
	result.z = 0.0f;
	result.w = 1.0f;
	return result;
}

Quaternion QuaternionMath::Conjugate(const Quaternion& q) {
	Quaternion result;
	result.x = -q.x;
	result.y = -q.y;
	result.z = -q.z;
	result.w = q.w;
	return result;
}

float QuaternionMath::Norm(const Quaternion& q) {
	float result = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	return result;
}

Quaternion QuaternionMath::Normalize(const Quaternion& q) {
	Quaternion result;
	float norm = Norm(q);
	result.x = q.x / norm;
	result.y = q.y / norm;
	result.z = q.z / norm;
	result.w = q.w / norm;
	return result;
}

Quaternion QuaternionMath::Inverse(const Quaternion& q) {
	Quaternion result;
	float norm = Norm(q);
	Quaternion conjugate = Conjugate(q);
	result.x = conjugate.x / powf(norm, 2);
	result.y = conjugate.y / powf(norm, 2);
	result.z = conjugate.z / powf(norm, 2);
	result.w = conjugate.w / powf(norm, 2);
	return result;
}

Quaternion QuaternionMath::MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	Quaternion result;
	float halfAngle = angle / 2.0f;
	float sinHalfAngle = sinf(halfAngle);
	result.x = axis.x * sinHalfAngle;
	result.y = axis.y * sinHalfAngle;
	result.z = axis.z * sinHalfAngle;
	result.w = cosf(halfAngle);
	return result;
}

Quaternion QuaternionMath::LookRotation(const Vector3& forward, const Vector3& up) {
	Vector3 zAxis = Vector3Math::Normalize(forward);
	Vector3 xAxis = Vector3Math::Normalize(Vector3Math::Cross(up, zAxis));
	Vector3 yAxis = Vector3Math::Cross(zAxis, xAxis);

	// 回転行列を作成
	Matrix4x4 rotationMatrix = {
		xAxis.x, yAxis.x, zAxis.x, 0.0f,
		xAxis.y, yAxis.y, zAxis.y, 0.0f,
		xAxis.z, yAxis.z, zAxis.z, 0.0f,
		0.0f,    0.0f,    0.0f,    1.0f
	};

	// 回転行列からクォータニオンを作成
	return QuaternionMath::FromMatrix(rotationMatrix);
}

Quaternion QuaternionMath::FromMatrix(const Matrix4x4& m) {
	Quaternion q;
	float trace = m.m[0][0] + m.m[1][1] + m.m[2][2];

	if (trace > 0.0f) {
		float s = sqrtf(trace + 1.0f) * 2.0f; // 4 * q.w
		q.w = 0.25f * s;
		q.x = (m.m[2][1] - m.m[1][2]) / s;
		q.y = (m.m[0][2] - m.m[2][0]) / s;
		q.z = (m.m[1][0] - m.m[0][1]) / s;
	} else {
		if (m.m[0][0] > m.m[1][1] && m.m[0][0] > m.m[2][2]) {
			float s = sqrtf(1.0f + m.m[0][0] - m.m[1][1] - m.m[2][2]) * 2.0f;
			q.w = (m.m[2][1] - m.m[1][2]) / s;
			q.x = 0.25f * s;
			q.y = (m.m[0][1] + m.m[1][0]) / s;
			q.z = (m.m[0][2] + m.m[2][0]) / s;
		} else if (m.m[1][1] > m.m[2][2]) {
			float s = sqrtf(1.0f + m.m[1][1] - m.m[0][0] - m.m[2][2]) * 2.0f;
			q.w = (m.m[0][2] - m.m[2][0]) / s;
			q.x = (m.m[0][1] + m.m[1][0]) / s;
			q.y = 0.25f * s;
			q.z = (m.m[1][2] + m.m[2][1]) / s;
		} else {
			float s = sqrtf(1.0f + m.m[2][2] - m.m[0][0] - m.m[1][1]) * 2.0f;
			q.w = (m.m[1][0] - m.m[0][1]) / s;
			q.x = (m.m[0][2] + m.m[2][0]) / s;
			q.y = (m.m[1][2] + m.m[2][1]) / s;
			q.z = 0.25f * s;
		}
	}

	return q;
}

Vector3 QuaternionMath::RotateVector(const Vector3& vector, const Quaternion& quaternion) {
	Vector3 result;
	Quaternion q = quaternion;
	Quaternion v = { vector.x,vector.y,vector.z,0.0f };
	Quaternion qConjugate = Conjugate(q);
	Quaternion v1 = Multiply(q, v);
	Quaternion v2 = Multiply(v1, qConjugate);
	result.x = v2.x;
	result.y = v2.y;
	result.z = v2.z;
	return result;
}

Vector3 QuaternionMath::toEuler(const Quaternion& q) {
	Vector3 angles;
	// X軸（ピッチ）
	float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	angles.x = std::atan2(sinr_cosp, cosr_cosp);

	// Y軸（ヨー）
	float sinp = 2 * (q.w * q.y - q.z * q.x);
	if (std::abs(sinp) >= 1)
		angles.y = std::copysign(std::numbers::pi_v<float> / 2, sinp); // クランプ（ジンバルロック）
	else
		angles.y = std::asin(sinp);

	// Z軸（ロール）
	float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	angles.z = std::atan2(siny_cosp, cosy_cosp);

	return angles;
}

Quaternion operator*(const Quaternion& q, float s) {
	return QuaternionMath::Multiply(q, s);
}

Quaternion operator*(float s, const Quaternion& q) {
	return q * s;
}

Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs) {
	return QuaternionMath::Multiply(lhs, rhs);
}

Quaternion operator/(const Quaternion& q, float s) {
	return { q.x / s, q.y / s, q.z / s, q.w / s };
}

Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs) {
	return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w };
}

Quaternion operator-(const Quaternion& lhs, const Quaternion& rhs) {
	return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w };
}

Quaternion operator-(const Quaternion& lhs) {
	return { -lhs.x, -lhs.y, -lhs.z, -lhs.w };
}
